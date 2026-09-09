param(
	[string] $BuildDirectory,
	[string] $CMake,
	[string] $SignTool,
	[string] $Generator,
	[ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
	[string] $Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$source = Split-Path -Parent $PSScriptRoot
if (-not $BuildDirectory) { $BuildDirectory = Join-Path $source 'build\signed-tests' }
$build = [System.IO.Path]::GetFullPath($BuildDirectory)
if ($build.TrimEnd('\') -eq $source.TrimEnd('\')) { throw 'Choose a separate build directory.' }

function Invoke-Checked {
	param([string] $Command, [string[]] $Arguments)
	& $Command @Arguments
	if ($LASTEXITCODE -ne 0) { throw "Command failed with exit code ${LASTEXITCODE}: $Command" }
}

if (-not $CMake) {
	$command = Get-Command cmake.exe -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
	if ($command) {
		$CMake = $command.Source
	} else {
		$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
		if (Test-Path -LiteralPath $vswhere -PathType Leaf) {
			$CMake = & $vswhere -latest -products '*' -find 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' |
				Select-Object -First 1
		}
	}
}
if (-not $CMake) { throw 'CMake was not found. Supply -CMake with its full path.' }
$CMake = (Resolve-Path -LiteralPath $CMake).Path
$ctest = Join-Path (Split-Path -Parent $CMake) 'ctest.exe'
if (-not (Test-Path -LiteralPath $ctest -PathType Leaf)) { throw 'CTest was not found beside CMake.' }

if (-not $SignTool) {
	$command = Get-Command signtool.exe -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
	if ($command) {
		$SignTool = $command.Source
	} else {
		$kits = Get-ItemProperty -LiteralPath 'HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots' -Name KitsRoot10
		$versions = Get-ChildItem -LiteralPath (Join-Path $kits.KitsRoot10 'bin') -Directory |
			Where-Object Name -Match '^10\.\d+\.\d+\.\d+$' | Sort-Object { [version] $_.Name } -Descending
		foreach ($version in $versions) {
			$candidate = Join-Path $version.FullName 'x64\signtool.exe'
			if (Test-Path -LiteralPath $candidate -PathType Leaf) { $SignTool = $candidate; break }
		}
	}
}
if (-not $SignTool) { throw 'Windows SDK SignTool was not found. Supply -SignTool with its full path.' }
$SignTool = (Resolve-Path -LiteralPath $SignTool).Path
New-Item -ItemType Directory -Path $build -Force | Out-Null

function Get-TestKeyHash($Certificate) {
	$sha256 = [System.Security.Cryptography.SHA256]::Create()
	try { $sha256.ComputeHash($Certificate.GetPublicKey()) } finally { $sha256.Dispose() }
}

$certificates = @()
try {
	foreach ($role in @('primary', 'other')) {
		$subject = 'CN=SFSE-MCP isolated tests ' + $role + ' ' + [Guid]::NewGuid().ToString('N')
		$certificates += New-SelfSignedCertificate -Type CodeSigningCert -Subject $subject `
			-FriendlyName 'SFSE-MCP temporary test signing' -CertStoreLocation 'Cert:\CurrentUser\My' `
			-Provider 'Microsoft Software Key Storage Provider' -KeyAlgorithm RSA -KeyLength 3072 `
			-HashAlgorithm SHA256 -KeyExportPolicy NonExportable -KeyUsage DigitalSignature `
			-NotBefore (Get-Date).AddMinutes(-5) -NotAfter (Get-Date).AddDays(1)
	}
	$certificate = $certificates[0]
	$keyHash = Get-TestKeyHash $certificate
	$keyHex = -join ($keyHash | ForEach-Object { $_.ToString('X2') })
	$initializer = ($keyHash | ForEach-Object { '0x' + $_.ToString('X2') }) -join ', '
	$keyHeader = Join-Path $build 'test-signing-key.hpp'
	$header = @"
#pragma once
#include <SFSEMCP/detail/Authenticode.hpp>
namespace SFSEMCP::detail {
inline constexpr SigningKeyHash ReleaseSigningKey{ $initializer };
}
"@
	[System.IO.File]::WriteAllText($keyHeader, $header, [System.Text.UTF8Encoding]::new($false))
	$publicInfo = [ordered]@{ Subject = $certificate.Subject; CertificateThumbprint = $certificate.Thumbprint; OtherCertificateThumbprint = $certificates[1].Thumbprint; PublicKeySha256 = $keyHex }
	[System.IO.File]::WriteAllText((Join-Path $build 'test-signing-public.json'), ($publicInfo | ConvertTo-Json),
		[System.Text.UTF8Encoding]::new($false))
	$configure = @('-S', $source, '-B', $build, '-DBUILD_TESTING=ON', "-DSFSEMCP_TEST_SIGNING_KEY_HEADER=$keyHeader")
	if ($Generator) { $configure += @('-G', $Generator) }
	if (-not $Generator -or $Generator -like 'Visual Studio*') { $configure += @('-A', 'x64') }
	Invoke-Checked $CMake $configure
	Invoke-Checked $CMake @('--build', $build, '--config', $Configuration)
	$artifacts = Get-Content -LiteralPath (Join-Path $build "tests\signed-tests-$Configuration.json") -Raw | ConvertFrom-Json
	foreach ($dll in @($artifacts.modern, $artifacts.legacy)) {
		Invoke-Checked $SignTool @('sign', '/q', '/fd', 'SHA256', '/sha1', $certificate.Thumbprint, '/s', 'My', $dll)
		Invoke-Checked $artifacts.verifier @($dll, $keyHex)
	}
	Invoke-Checked $SignTool @('sign', '/q', '/fd', 'SHA256', '/sha1', $certificates[1].Thumbprint, '/s', 'My', $artifacts.wrong)
	$otherHash = -join ((Get-TestKeyHash $certificates[1]) | ForEach-Object { $_.ToString('X2') })
	Invoke-Checked $artifacts.verifier @($artifacts.wrong, $otherHash)
	& (Join-Path $PSScriptRoot 'create_tampered_fixture.ps1') -BuiltDll $artifacts.modern -OutputDll $artifacts.tampered
	Invoke-Checked $ctest @('--test-dir', $build, '-C', $Configuration, '--output-on-failure')
} finally {
	$cleanupErrors = @()
	foreach ($testCertificate in $certificates) {
		# Finish cleaning other test keys even if one cleanup operation fails.
		try {
			$certificatePath = 'Cert:\CurrentUser\My\' + $testCertificate.Thumbprint
			$expectedSubject = $testCertificate.Subject
			$rsa = [System.Security.Cryptography.X509Certificates.RSACertificateExtensions]::GetRSAPrivateKey($testCertificate)
			try { $keyName = $rsa.Key.KeyName; $keyProvider = $rsa.Key.Provider } finally { $rsa.Dispose() }
			$testCertificate.Dispose()
			$stored = Get-Item -LiteralPath $certificatePath
			if ($stored.Subject -cne $expectedSubject) { throw 'Temporary certificate identity changed; cleanup stopped.' }
			$stored.Dispose()
			Remove-Item -LiteralPath $certificatePath -DeleteKey -Force
			if ((Test-Path -LiteralPath $certificatePath) -or
				[System.Security.Cryptography.CngKey]::Exists($keyName, $keyProvider)) {
				throw 'Temporary signing certificate or private key was not removed.'
			}
		} catch {
			$cleanupErrors += $_.Exception.Message
		}
	}
	if ($cleanupErrors.Count) { throw ($cleanupErrors -join [Environment]::NewLine) }
	Write-Host 'Removed the temporary signing certificates and private keys.'
}
