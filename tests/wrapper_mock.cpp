#include <SFSEMCP/SFSEMenuFramework.hpp>

#include <Windows.h>

#include <cstdint>
#include <cstring>

namespace {
bool __stdcall OnInput(RE::InputEvent *) { return false; }

void __stdcall RenderPanel() {}
} // namespace

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

    using ReadStringFunction = const char *(*)();
    const auto readLastSection = GetMenuFrameworkFunction<ReadStringFunction>("GetLastSectionPath");
    const auto readLastRenamePath = GetMenuFrameworkFunction<ReadStringFunction>("GetLastRenamePath");
    const auto readLastRenameName = GetMenuFrameworkFunction<ReadStringFunction>("GetLastRenameName");
    const auto readLastDelete = GetMenuFrameworkFunction<ReadStringFunction>("GetLastDeletePath");
    if (!readLastSection || !readLastRenamePath || !readLastRenameName || !readLastDelete) {
        return 20;
    }

    SFSEMenuFramework::SetSection("Root");
    SFSEMenuFramework::AddSectionItem("Child\\/Leaf", &RenderPanel);
    if (std::strcmp(readLastSection(), "Root/Child\\/Leaf") != 0) {
        return 21;
    }
    SFSEMenuFramework::FullPathAddSectionItem("Absolute\\/Root/Page", &RenderPanel);
    if (std::strcmp(readLastSection(), "Absolute\\/Root/Page") != 0) {
        return 22;
    }
    if (!SFSEMenuFramework::RenameSection("Root/Old", "New\\/Name") ||
        std::strcmp(readLastRenamePath(), "Root/Old") != 0 || std::strcmp(readLastRenameName(), "New\\/Name") != 0) {
        return 23;
    }
    if (!SFSEMenuFramework::DeleteSection("Root/New\\/Name") || std::strcmp(readLastDelete(), "Root/New\\/Name") != 0) {
        return 24;
    }
    if (SFSEMenuFramework::RenameSection("Reject/Rename", "Name") ||
        SFSEMenuFramework::DeleteSection("Reject/Delete")) {
        return 25;
    }
    if (SFSEMenuFramework::GetMenuFrameworkAPIVersion() != 1) {
        return 26;
    }

    char formatted[32]{};
    const int formattedLength = ImGuiMCP::ImFormatString(formatted, sizeof(formatted), "%s %d", "value", 42);
    if (formattedLength != 8 || std::strcmp(formatted, "value 42") != 0) {
        return 27;
    }
    if (::GetProcAddress(module, "RegisterInputEvent") == nullptr) {
        return 30;
    }
    if (::GetProcAddress(module, "RegisterInpoutEvent") != nullptr) {
        return 31;
    }

    auto *inputEvent = SFSEMenuFramework::AddInputEvent(&OnInput);
    if (inputEvent == nullptr || !inputEvent->IsRegistered()) {
        return 32;
    }
    delete inputEvent;

    using LastUnregisteredFunction = std::uint64_t (*)();
    const auto lastUnregistered = GetMenuFrameworkFunction<LastUnregisteredFunction>("GetLastUnregisteredInputEvent");
    if (lastUnregistered == nullptr || lastUnregistered() != 73) {
        return 33;
    }

    return 0;
}
