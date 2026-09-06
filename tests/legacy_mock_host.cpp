#include <string>

namespace {
std::string lastSectionPath;
}

using RenderFunction = void(__stdcall *)();

extern "C" __declspec(dllexport) void AddSectionItem(const char *a_path, const RenderFunction a_renderer) {
    lastSectionPath = a_path && a_renderer ? a_path : "";
}

extern "C" __declspec(dllexport) const char *GetLastSectionPath() { return lastSectionPath.c_str(); }
