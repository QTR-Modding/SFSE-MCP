#include <SFSEMCP/SFSEMenuFramework.hpp>

#include <Windows.h>

#include <cstring>

namespace {
void __stdcall RenderPanel() {}
} // namespace

int wmain(const int a_argumentCount, wchar_t **a_arguments) {
    if (a_argumentCount != 2) {
        return 1;
    }

    const auto module = ::LoadLibraryExW(a_arguments[1], nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!module) {
        return 2;
    }

    using ReadStringFunction = const char *(*)();
    const auto readLastSection = GetMenuFrameworkFunction<ReadStringFunction>("GetLastSectionPath");
    if (!readLastSection) {
        return 3;
    }

    SFSEMenuFramework::FullPathAddSectionItem("Legacy/Full\\/path", &RenderPanel);
    if (std::strcmp(readLastSection(), "Legacy/Full\\/path") != 0) {
        return 4;
    }
    if (SFSEMenuFramework::RenameSection("Legacy/Old", "New")) {
        return 5;
    }
    if (SFSEMenuFramework::DeleteSection("Legacy/Old")) {
        return 6;
    }
    if (SFSEMenuFramework::GetMenuFrameworkAPIVersion() != 0) {
        return 7;
    }

    return 0;
}
