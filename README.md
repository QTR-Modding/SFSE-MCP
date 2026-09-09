# SFSE-MCP

SFSE-MCP is the header-only C++ client SDK for SFSE Menu Framework.

It is a close Starfield adaptation of QTR SKSE-MCP at commit
`996e0215e98add566dbb98234bc81b8fe82abb52`. The public API keeps the
intentional SKSE-MCP names and calling model while replacing SKSE-specific
names, paths, and packaging with their SFSE equivalents.

The SDK contains no Dear ImGui implementation, static library, or transitive
ImGui dependency. Every `ImGuiMCP` call resolves its corresponding export from
the authenticated, loaded `SFSEMenuFramework.dll`. The SDK enumerates loaded
modules, verifies the pinned signing key and loaded executable bytes, and reads
the verified export table directly. Detected replacements or modified exports
produce an error and stop the process before calling that provider. This does
not prevent arbitrary in-process tampering with the client, imports, mutable
runtime state, or code changed after verification.

## Use

```cpp
#include <SFSEMCP/SFSEMenuFramework.hpp>

void __stdcall DrawSettings() {
    ImGuiMCP::TextUnformatted("Hello from Starfield");
}

void RegisterSettings() {
    SFSEMenuFramework::SetSection("Example");
    SFSEMenuFramework::AddSectionItem("Settings", &DrawSettings);
}
```

Menu paths use `/` between sections and `\/` for a literal slash in a section
name. `FullPathAddSectionItem` bypasses the `SetSection` prefix and uses the
original `AddSectionItem` host export. API version 1 adds runtime
`RenameSection` and `DeleteSection`; both return `false` for an older host and
for invalid, missing, or colliding paths. Use `GetMenuFrameworkAPIVersion()`
before relying on those mutation calls; `0` means that query export is absent.

A typical SKSE-MCP client port changes:

- `#include <SKSEMCP/SKSEMenuFramework.hpp>` to
  `#include <SFSEMCP/SFSEMenuFramework.hpp>`
- `SKSEMenuFramework` to `SFSEMenuFramework`
- SKSE/SFSE project dependencies as required by the game plugin

`ImGuiMCP`, `FontAwesome`, callback signatures, manager names, and
intentional exported function names remain the same. The port deliberately
corrects obvious source defects instead of reproducing them:

- `RegisterInputEvent` and `*Function` use their correct spellings.
- Registration handles start at zero, failed registrations return `nullptr`,
  and owning registration wrappers cannot be copied.
- `AddEvent(callback)` restores the documented default priority of `0.0F`.
- `ImFormatString` returns the host formatter result instead of discarding it.
- The window boolean is named `blockUserInput`, matching the state it controls.
- `IsAnyBlockingWindowOpen` is the corrected name; the original
  `IsAnyBlockingWindowOpened` remains as a source-compatible alias.
- `AddWindow` and `AddWindowWithView` check returned pointers before using
  them. The Starfield host keeps `AddWindowWithView` source-compatible and
  creates a normal framework window; its `viewName` is currently ignored
  because the pinned Skyrim host never implemented a view-specific export.

Pointers returned by `AddWindow`, `AddWindowWithView`, and `GetMainWindow` are
borrowed from the framework, remain stable until process exit, and must not be
deleted by the client.

Call framework APIs from SFSE load callbacks or later, not `DllMain` or global
initializers: first use performs signature verification outside the loader lock.
`IsInstalled()` means a verified framework is loaded, not merely present on disk.
Missing providers remain retryable; optional exports retain their existing
fallbacks. These trust checks change binding, not callback signatures or ownership.
The SDK remains MIT and header-only. MSVC links Windows `Crypt32` automatically.
Rebuilt forks can select their own public-key header with
`SFSEMCP_SIGNING_KEY_HEADER`; shipped clients have no runtime bypass switch.

## Single-header download

Vcpkg is optional. The SDK ZIP contains one self-contained
`SFSEMCP/SFSEMenuFramework.hpp`. Copy the `SFSEMCP` folder into your project's
include directory, then use the same include and API shown above. It requires
C++23 and Windows x64; no signing tools or private key are needed. Keep its
embedded license notices. Use either this download or the multi-file SDK,
not both in the same project.

To generate the ZIP from the maintained source headers:

```powershell
./scripts/Package-Sdk.ps1
```

The versioned archive is written to `build/packages`. Generated headers and
archives are not committed.

## CMake

Copy `cmake/ports/sfse-mcp` into the same path in the client project, add
`sfse-mcp` to its `vcpkg.json` dependencies, and add or merge this into the
client's `vcpkg-configuration.json`:

```json
{
  "overlay-ports": ["cmake/ports"]
}
```

Then install with:

```powershell
vcpkg install --triplet x64-windows
```

While the repository is private, Git must already be authenticated for an
account with access.

Then consume the installed header-only target normally:

```cmake
find_package(SFSE-MCP CONFIG REQUIRED)
target_link_libraries(my_plugin PRIVATE SFSE-MCP::SFSE-MCP)
```

`SFSE-MCP::SFSE-MCP` is an `INTERFACE` target. It supplies only the SDK include
directory and the C++23 requirement.

## Current limitations

Lifecycle event callbacks run outside an active ImGui frame and must not call
`ImGuiMCP`. Use page, window, or HUD callbacks for ImGui drawing.

`LoadTexture` and `DisposeTexture` are retained for SKSE-MCP source
compatibility, but the current Starfield host does not export them. Loading
therefore returns a null texture and disposal is a no-op.

To build the SDK checks:

```powershell
./tests/run_signed_tests.ps1
./tests/run_signed_tests.ps1 -Standalone
```

SFSE-MCP is available under the MIT License. See `THIRD_PARTY_NOTICES` for the
pinned SKSE-MCP and generated ImGui API provenance.
