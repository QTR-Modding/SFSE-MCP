# SFSE-MCP

Add an in-game settings menu to your C++ SFSE mod with
[SFSE Menu Framework](https://github.com/QTR-Modding/SFSE-Menu-Framework).
Based on SkyrimThiago's SKSE-MCP.

## 1. Add the SDK

Use an existing **C++23, Windows x64 SFSE project**. Choose either the header
or [vcpkg](#vcpkg); you don't need both.

### Standalone header

Download the SDK from the framework's Nexus page under **Miscellaneous Files**.
Copy its `SFSEMCP` folder into your project's include directory, keeping the
included license notices. You can then include:

```cpp
#include <SFSEMCP/SFSEMenuFramework.hpp>
```

### Vcpkg

1. Copy both port folders from [cmake/ports](cmake/ports) into your project's
   `cmake/ports`.
2. Add these entries to the `dependencies` array in your `vcpkg.json`:

   ```json
   "sfse-mcp",
   { "name": "clib-utils-qtr", "default-features": false }
   ```

   Keep `default-features: false` so your Starfield project doesn't pull in
   QTR Utils' Skyrim dependencies.
3. Add `"overlay-ports": ["cmake/ports"]` to `vcpkg-configuration.json`.
4. Run `vcpkg install --triplet x64-windows`.
5. In your CMake project, replace `my_plugin` with your target name:

   ```cmake
   find_package(SFSE-MCP CONFIG REQUIRED)
   target_link_libraries(my_plugin PRIVATE SFSE-MCP::SFSE-MCP)
   ```

You don't need to compile or link your own ImGui library for this menu.

## 2. Add your menu

Create `Menu.h`:

```cpp
#pragma once

namespace Menu
{
    void Register();
}
```

Create `Menu.cpp` and add it to your build:

```cpp
#include "Menu.h"
#include <SFSEMCP/SFSEMenuFramework.hpp>

namespace
{
    bool enabled = true;
    float strength = 1.0f;

    void __stdcall DrawSettings()
    {
        ImGuiMCP::TextUnformatted("Hello from my mod!");
        ImGuiMCP::Checkbox("Enabled", &enabled);
        ImGuiMCP::SliderFloat("Strength", &strength, 0.0f, 2.0f);

        if (ImGuiMCP::Button("Reset"))
        {
            enabled = true;
            strength = 1.0f;
        }
    }
}

void Menu::Register()
{
    if (!SFSEMenuFramework::IsInstalled())
    {
        return;
    }

    SFSEMenuFramework::SetSection("My Mod");
    SFSEMenuFramework::AddSectionItem("Settings", &DrawSettings);
}
```

`My Mod` is the name in the left panel. `Settings` is the page underneath it.
The framework calls `DrawSettings` each frame while that page is visible.

The checkbox and slider change the variables you pass to them. `Button`
returns `true` when pressed. Replace these example variables with your mod's
settings; saving them to disk is up to your mod.

## 3. Register it with SFSE

**The page won't appear until your plugin calls `Menu::Register()`.**

If your plugin already has an SFSE message listener, include `Menu.h` and add
this to its existing `kPostLoad` handler:

```cpp
Menu::Register();
```

If it doesn't have a listener, add this to `plugin.cpp`:

```cpp
#include "Menu.h"
#include <SFSE/SFSE.h>

namespace
{
    void OnSFSEMessage(SFSE::MessagingInterface::Message* message)
    {
        if (message &&
            message->type == SFSE::MessagingInterface::kPostLoad)
        {
            Menu::Register();
        }
    }
}
```

Then register that listener in your **existing plugin entrypoint**, after
`SFSE::Init(...)`:

```cpp
const auto* messaging = SFSE::GetMessagingInterface();
if (!messaging || !messaging->RegisterListener(OnSFSEMessage))
{
    return false;
}
```

Keep the rest of your entrypoint and its final `return true`. Don't add a
second entrypoint or a second registration if you already have a listener.
See the example mod's [complete plugin.cpp](https://github.com/QTR-Modding/SFSE-Menu-Framework-Example/blob/main/src/plugin.cpp)
for this wiring in a working project.

## 4. See it in game

1. Install SFSE and SFSE Menu Framework in your game setup.
2. Build your mod and install its DLL under `Data/SFSE/Plugins` (or that path
   inside your mod manager's mod folder). Enable both mods.
3. Launch the game through SFSE.
4. Press **F1** and select **My Mod > Settings**.

You should see the text, checkbox, slider and Reset button. If you changed the
framework's menu binding, use that key instead.

If the panel opens but your mod is missing, check that your DLL loaded and
that your `kPostLoad` handler actually calls `Menu::Register()`.

## More examples

- [Menu registration, settings pages and separate windows](https://github.com/QTR-Modding/SFSE-Menu-Framework-Example/blob/main/src/Menu.cpp)
- [Input listeners and persistent HUDs](https://github.com/QTR-Modding/SFSE-Menu-Framework-Example/blob/main/src/InputHudDemo.cpp)
- [Fonts](https://github.com/QTR-Modding/SFSE-Menu-Framework-Example/blob/main/src/FontDemo.cpp)
- [API reference and porting from SKSE-MCP](docs/API.md)

## License

[MIT](LICENSE). Using the SDK does not require your mod to adopt the framework's
GPL license. Keep the included [third-party notices](THIRD_PARTY_NOTICES).
