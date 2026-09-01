#include <cstring>
#include <cstdint>

namespace RE {
class InputEvent;
}

struct ImVec2 {
    float x;
    float y;
};

extern "C" __declspec(dllexport) bool igButton(const char *label, const ImVec2 size) {
    return label != nullptr && std::strcmp(label, "MCP dispatch") == 0 &&
           size.x == 42.0F && size.y == 24.0F;
}

extern "C" __declspec(dllexport) bool IsHotkeyEnabled() {
    return true;
}

using InputEventCallback = bool(__stdcall *)(RE::InputEvent *);

namespace {
std::uint64_t LastUnregisteredInputEvent = 0;
}

extern "C" __declspec(dllexport) std::int64_t RegisterInputEvent(
    const InputEventCallback callback) {
    return callback != nullptr ? 73 : 0;
}

extern "C" __declspec(dllexport) void UnregisterInputEvent(const std::uint64_t id) {
    LastUnregisteredInputEvent = id;
}

extern "C" __declspec(dllexport) std::uint64_t GetLastUnregisteredInputEvent() {
    return LastUnregisteredInputEvent;
}
