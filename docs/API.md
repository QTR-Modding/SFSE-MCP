# API reference

For your first menu, start with the [README](../README.md).

## Callbacks

Draw with `ImGuiMCP` only in page, window or HUD callbacks. Lifecycle and input
callbacks run outside the drawing frame. Do not let exceptions escape a callback.

`AddEvent` handles menu open/close and before/after-render events; higher
priorities run first. `AddInputEvent` can consume an event by returning `true`.
`AddHudElement` draws before windows, including while the panel is closed.
Delete these registration objects to unregister their callbacks.

## Windows and fonts

Window pointers from `AddWindow`, `AddWindowWithView` and `GetMainWindow`
belong to the framework. They remain valid until exit; do not delete them.
Use their atomic `IsOpen` and `BlockUserInput` fields to control the window.
Check registration results for `nullptr`.

Live font changes invalidate cached `ImFont*` pointers. Use the named font
helpers inside your render callbacks. `PushFont` accepts a filename or stem;
pair a successful Font Awesome push with `FontAwesome::Pop`.

## Menu paths and porting

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
