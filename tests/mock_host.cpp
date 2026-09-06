#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace RE {
class InputEvent;
}

struct ImVec2 {
    float x;
    float y;
};

extern "C" __declspec(dllexport) bool igButton(const char *label, const ImVec2 size) {
    return label != nullptr && std::strcmp(label, "MCP dispatch") == 0 && size.x == 42.0F && size.y == 24.0F;
}

extern "C" __declspec(dllexport) bool IsHotkeyEnabled() { return true; }

using InputEventCallback = bool(__stdcall *)(RE::InputEvent *);

namespace {
std::uint64_t LastUnregisteredInputEvent = 0;
std::string LastSectionPath;
std::string LastRenamePath;
std::string LastRenameName;
std::string LastDeletePath;
} // namespace

using RenderFunction = void(__stdcall *)();

extern "C" __declspec(dllexport) void AddSectionItem(const char *path, const RenderFunction renderer) {
    LastSectionPath = path ? path : "";
    if (renderer == nullptr) {
        LastSectionPath.clear();
    }
}

extern "C" __declspec(dllexport) bool RenameSection(const char *path, const char *newName) {
    LastRenamePath = path ? path : "";
    LastRenameName = newName ? newName : "";
    return path != nullptr && newName != nullptr && std::strcmp(path, "Reject/Rename") != 0;
}

extern "C" __declspec(dllexport) bool DeleteSection(const char *path) {
    LastDeletePath = path ? path : "";
    return path != nullptr && std::strcmp(path, "Reject/Delete") != 0;
}

extern "C" __declspec(dllexport) std::uint32_t GetMenuFrameworkAPIVersion() { return 1; }

extern "C" __declspec(dllexport) const char *GetLastSectionPath() { return LastSectionPath.c_str(); }

extern "C" __declspec(dllexport) const char *GetLastRenamePath() { return LastRenamePath.c_str(); }

extern "C" __declspec(dllexport) const char *GetLastRenameName() { return LastRenameName.c_str(); }

extern "C" __declspec(dllexport) const char *GetLastDeletePath() { return LastDeletePath.c_str(); }

extern "C" __declspec(dllexport) int igImFormatStringV(char *buffer, const std::size_t bufferSize, const char *format,
                                                       va_list arguments) {
    return std::vsnprintf(buffer, bufferSize, format, arguments);
}

extern "C" __declspec(dllexport) std::int64_t RegisterInputEvent(const InputEventCallback callback) {
    return callback != nullptr ? 73 : 0;
}

extern "C" __declspec(dllexport) void UnregisterInputEvent(const std::uint64_t id) { LastUnregisteredInputEvent = id; }

extern "C" __declspec(dllexport) std::uint64_t GetLastUnregisteredInputEvent() { return LastUnregisteredInputEvent; }
