#include <SFSEMCP/SFSEMenuFramework.hpp>

#include <Windows.h>

#include <cstdint>

namespace {
bool __stdcall OnInput(RE::InputEvent *) {
    return false;
}
}

int wmain(const int argc, wchar_t **argv) {
    if (argc != 2) {
        return 10;
    }

    const HMODULE module = ::LoadLibraryW(argv[1]);
    if (module == nullptr) {
        return 11;
    }
    if (GetMenuFrameworkModule() != module) {
        return 12;
    }

    using RawButton = bool (*)(const char *, ImGuiMCP::ImVec2);
    const auto rawButton = GetMenuFrameworkFunction<RawButton>("igButton");
    if (rawButton == nullptr) {
        return 13;
    }
    if (!ImGuiMCP::Button("MCP dispatch", ImGuiMCP::ImVec2(42.0F, 24.0F))) {
        return 14;
    }
    if (!SFSEMenuFramework::IsHotkeyEnabled()) {
        return 15;
    }
    if (::GetProcAddress(module, "RegisterInputEvent") == nullptr) {
        return 16;
    }
    if (::GetProcAddress(module, "RegisterInpoutEvent") != nullptr) {
        return 17;
    }

    auto *inputEvent = SFSEMenuFramework::AddInputEvent(&OnInput);
    if (inputEvent == nullptr || !inputEvent->IsRegistered()) {
        return 18;
    }
    delete inputEvent;

    using LastUnregisteredFunction = std::uint64_t (*)();
    const auto lastUnregistered =
        GetMenuFrameworkFunction<LastUnregisteredFunction>("GetLastUnregisteredInputEvent");
    if (lastUnregistered == nullptr || lastUnregistered() != 73) {
        return 19;
    }

    return 0;
}
