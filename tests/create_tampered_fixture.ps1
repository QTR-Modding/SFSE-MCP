param(
	[Parameter(Mandatory)] [string] $BuiltDll,
	[Parameter(Mandatory)] [string] $OutputDll
)

$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $BuiltDll).Path
$destination = [IO.Path]::GetFullPath($OutputDll)
if ($source -eq $destination -or [IO.Path]::GetExtension($destination) -ne '.dll') {
	throw 'The fixture must be a separate output DLL.'
}
$data = [IO.File]::ReadAllBytes($source)
if ($data.Length -lt 64 -or [BitConverter]::ToUInt16($data, 0) -ne 0x5A4D) { throw 'Not a PE file.' }
$nt = [BitConverter]::ToInt32($data, 60)
if ($nt -lt 0 -or $nt -gt $data.Length - 264 -or [BitConverter]::ToUInt32($data, $nt) -ne 0x4550 -or
	[BitConverter]::ToUInt16($data, $nt + 4) -ne 0x8664) { throw 'Not an x64 PE image.' }
$sectionCount = [BitConverter]::ToUInt16($data, $nt + 6)
$sections = $nt + 24 + [BitConverter]::ToUInt16($data, $nt + 20)
if ($sectionCount -gt 96 -or $sections + 40 * $sectionCount -gt $data.Length) { throw 'Invalid section table.' }

function FileOffset([uint32] $rva) {
	for ($index = 0; $index -lt $sectionCount; $index++) {
		$section = $sections + 40 * $index
		$address = [BitConverter]::ToUInt32($data, $section + 12)
		$size = [BitConverter]::ToUInt32($data, $section + 16)
		if ($rva -ge $address -and $rva -lt [uint64]$address + $size) {
			$offset = [uint64]$rva - $address + [BitConverter]::ToUInt32($data, $section + 20)
			if ($offset -ge $data.Length) { throw 'RVA is outside the file.' }
			return [int]$offset
		}
	}
	throw 'RVA is not mapped.'
}

$exports = FileOffset ([BitConverter]::ToUInt32($data, $nt + 24 + 112))
$names = FileOffset ([BitConverter]::ToUInt32($data, $exports + 32))
$ordinals = FileOffset ([BitConverter]::ToUInt32($data, $exports + 36))
$functions = FileOffset ([BitConverter]::ToUInt32($data, $exports + 28))
$nameCount = [BitConverter]::ToUInt32($data, $exports + 24)
if ($nameCount -gt 65536) { throw 'Invalid export count.' }
$found = $false
for ($index = 0; $index -lt $nameCount; $index++) {
	$name = FileOffset ([BitConverter]::ToUInt32($data, $names + 4 * $index))
	if ([Text.Encoding]::ASCII.GetString($data, $name, 9) -cne "igButton`0") { continue }
	$ordinal = [BitConverter]::ToUInt16($data, $ordinals + 2 * $index)
	$target = FileOffset ([BitConverter]::ToUInt32($data, $functions + 4 * $ordinal))
	$data[$target] = $data[$target] -bxor 1
	$found = $true
	break
}
if (-not $found) { throw 'Test export not found.' }
New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
[IO.File]::WriteAllBytes($destination, $data)
Write-Host 'Created an isolated DLL with one changed instruction and its original signature.'
