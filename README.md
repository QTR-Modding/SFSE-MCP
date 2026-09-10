# SFSE-MCP

A header-only C++ SDK for adding menus to
[SFSE Menu Framework](https://github.com/QTR-Modding/SFSE-Menu-Framework).
Based on SkyrimThiago's SKSE-MCP. Your plugin calls `ImGuiMCP`; the framework
DLL runs ImGui.

Requires C++23 and Windows x64.

## Add it to your project

Choose one method; do not mix the single-header and multi-file SDK.

### Single-header ZIP

Copy the ZIP's `SFSEMCP` folder into your project's include directory.
It contains one self-contained `SFSEMenuFramework.hpp`, including the license
notices. No vcpkg, signing tools or private key is needed.

### Vcpkg

1. Copy both folders from [cmake/ports](cmake/ports) into your project's `cmake/ports`.
2. Add these dependencies to your `vcpkg.json`:

   ```json
   "dependencies": [
     "sfse-mcp",
     { "name": "clib-utils-qtr", "default-features": false }
   ]
   ```

   Keep the explicit base-only dependency: vcpkg can otherwise enable QTR's
   default Skyrim features through the transitive dependency.
3. Add `"overlay-ports": ["cmake/ports"]` to your `vcpkg-configuration.json`.
4. Run `vcpkg install --triplet x64-windows`.

Then add this to your CMake project:

```cmake
find_package(SFSE-MCP CONFIG REQUIRED)
target_link_libraries(my_plugin PRIVATE SFSE-MCP::SFSE-MCP)
```

This adds the headers and C++23 requirement, not an ImGui library.
While this repository is private, Git needs an account with access.

### Source checkout

Clone with `--recurse-submodules`, then add this repository with CMake's
`add_subdirectory`. For a manual build, add both `include` and
`lib/clib-utils-qtr/include` to your include paths. Only QTR Utils' signing
headers are used; no Skyrim modules are required.

## Add a settings page

```cpp
#include <SFSEMCP/SFSEMenuFramework.hpp>

void __stdcall DrawSettings() {
    ImGuiMCP::TextUnformatted("Hello from Starfield");
}

// Call from your SFSE kPostLoad listener.
void RegisterSettings() {
    SFSEMenuFramework::SetSection("My Mod");
    SFSEMenuFramework::AddSectionItem("Settings", &DrawSettings);
}
```

See the [example mod](https://github.com/QTR-Modding/SFSE-Menu-Framework-Example)
for windows, fonts, events, input listeners and HUD elements.

Call the API from SFSE load callbacks or later, not `DllMain` or global
initializers. Registering a menu early does not make game data ready.

## Framework verification

Before using a framework DLL, the SDK checks its signature, public key and
loaded code. A detected mismatch shows an error and exits Starfield.
If the framework is missing, calls can retry when it loads; `IsInstalled()`
means a verified DLL is loaded, not just present on disk.

This cannot stop every form of tampering by another plugin in the same process,
including changes made after verification. MSVC links Windows `Crypt32`
automatically. Forks can choose their own public-key header at compile time with
`SFSEMCP_SIGNING_KEY_HEADER`; there is no runtime switch to skip verification.

## API notes

<details>
<summary>Callbacks, ownership and compatibility</summary>

### Callbacks

Draw with `ImGuiMCP` only in page, window or HUD callbacks. Lifecycle and input
callbacks run outside the drawing frame. Do not let exceptions escape a callback.

`AddEvent` handles menu open/close and before/after-render events; higher
priorities run first. `AddInputEvent` can consume an event by returning `true`.
`AddHudElement` draws before windows, including while the panel is closed.
Delete these registration objects to unregister their callbacks.

### Windows and fonts

Window pointers from `AddWindow`, `AddWindowWithView` and `GetMainWindow`
belong to the framework. They remain valid until exit; do not delete them.
Use their atomic `IsOpen` and `BlockUserInput` fields to control the window.
Check registration results for `nullptr`.

Live font changes invalidate cached `ImFont*` pointers. Use the named font
helpers inside your render callbacks. `PushFont` accepts a filename or stem;
pair a successful Font Awesome push with `FontAwesome::Pop`.

### Menu paths and porting

Use `/` for nested sections and `\/` for a literal slash in a name.
`FullPathAddSectionItem` takes a full path without the `SetSection` prefix.
Check `GetMenuFrameworkAPIVersion() >= 1` before using `RenameSection` or
`DeleteSection`. They return `false` for unavailable exports or invalid,
missing or colliding paths.
Do not use `GetMenuFrameworkVersion()` to check capabilities.

For an SKSE-MCP port, change the include to `SFSEMCP/SFSEMenuFramework.hpp`,
the namespace to `SFSEMenuFramework`, and your SKSE dependencies to SFSE.
`ImGuiMCP`, `FontAwesome` and callback signatures keep the same calling style.
Corrected spellings include `RegisterInputEvent` and `*Function`.
Both `IsAnyBlockingWindowOpen` and its older `IsAnyBlockingWindowOpened`
alias work. Registration objects cannot be copied.

`AddWindowWithView` creates a normal window and ignores `viewName`.
`LoadTexture` returns a null texture and `DisposeTexture` does nothing:
framework wallpapers do not make these client texture functions available.

</details>

## Packaging and tests

Generate the single-header ZIP with `./scripts/Package-Sdk.ps1`.
It is written to `build/packages`; keep generated files out of source control.

To run the SDK checks with the source headers and the extracted ZIP:

```powershell
./tests/run_signed_tests.ps1
./tests/run_signed_tests.ps1 -Standalone
```

## License

[MIT](LICENSE). See [third-party notices](THIRD_PARTY_NOTICES) for the
SKSE-MCP, Dear ImGui and cimgui credits and source revisions.
