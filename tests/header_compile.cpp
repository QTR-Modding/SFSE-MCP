#include <SFSEMCP/SFSEMenuFramework.hpp>
#include <SFSEMCP/SFSEMenuFramework.hpp>

#include <type_traits>

namespace Framework = SFSEMenuFramework;
namespace MCP = SFSEMenuFramework::Model;

using ModuleFunction = HMODULE (*)();
using ResolveFunction = MCP::ActionFunction (*)(LPCSTR);
using RenderFunction = void(__stdcall *)();
using InputCallback = bool(__stdcall *)(RE::InputEvent *);
using HudCallback = void(__stdcall *)();
using EventCallback = void(__stdcall *)(MCP::EventType);
using ButtonFunction = bool (*)(const char *, const ImGuiMCP::ImVec2);

static_assert(std::is_same_v<decltype(&GetMenuFrameworkModule), ModuleFunction>);
static_assert(std::is_same_v<decltype(&GetMenuFrameworkFunction<MCP::ActionFunction>), ResolveFunction>);
static_assert(std::is_same_v<std::underlying_type_t<MCP::EventType>, std::uint32_t>);
static_assert(std::is_same_v<MCP::RenderFunction, RenderFunction>);
static_assert(std::is_same_v<MCP::InputEventCallback, InputCallback>);
static_assert(std::is_same_v<MCP::HudElementCallback, HudCallback>);
static_assert(std::is_same_v<MCP::EventCallback, EventCallback>);
static_assert(!std::is_copy_constructible_v<MCP::Event>);
static_assert(!std::is_copy_assignable_v<MCP::Event>);
static_assert(!std::is_copy_constructible_v<MCP::InputEvent>);
static_assert(!std::is_copy_assignable_v<MCP::InputEvent>);
static_assert(!std::is_copy_constructible_v<MCP::HudElement>);
static_assert(!std::is_copy_assignable_v<MCP::HudElement>);
static_assert(!std::is_copy_constructible_v<ImGuiMCP::ImGuiTextFilter>);
static_assert(!std::is_copy_assignable_v<ImGuiMCP::ImGuiTextFilter>);
static_assert(std::is_same_v<decltype(&ImGuiMCP::Button), ButtonFunction>);
static_assert(std::is_same_v<decltype(&Framework::SetSection), void (*)(std::string)>);
static_assert(std::is_same_v<decltype(&Framework::AddSectionItem), void (*)(std::string, RenderFunction)>);
static_assert(std::is_same_v<decltype(&Framework::FullPathAddSectionItem), void (*)(std::string, RenderFunction)>);
static_assert(std::is_same_v<decltype(&Framework::RenameSection), bool (*)(std::string, std::string)>);
static_assert(std::is_same_v<decltype(&Framework::DeleteSection), bool (*)(std::string)>);
static_assert(std::is_same_v<decltype(&Framework::AddWindow), MCP::WindowInterface *(*)(RenderFunction, bool)>);
static_assert(std::is_same_v<decltype(&Framework::AddWindowWithView),
                             MCP::WindowInterface *(*)(RenderFunction, std::string, bool)>);
static_assert(std::is_same_v<decltype(&Framework::AddEvent), MCP::Event *(*)(EventCallback, float)>);
static_assert(std::is_constructible_v<MCP::Event, EventCallback>);
static_assert(std::is_same_v<decltype(&Framework::AddInputEvent), MCP::InputEvent *(*)(InputCallback)>);
static_assert(std::is_same_v<decltype(&Framework::AddHudElement), MCP::HudElement *(*)(HudCallback)>);
static_assert(std::is_same_v<decltype(&Framework::GetMenuFrameworkVersion), float (*)()>);
static_assert(std::is_same_v<decltype(&Framework::GetMenuFrameworkAPIVersion), std::uint32_t (*)()>);
static_assert(std::is_same_v<decltype(&ImGuiMCP::ImFormatString), int (*)(char *, size_t, const char *, ...)>);
static_assert(std::is_same_v<decltype(&Framework::IsAnyBlockingWindowOpen), bool (*)()>);
static_assert(std::is_same_v<decltype(&Framework::IsAnyBlockingWindowOpened), bool (*)()>);
static_assert(std::is_same_v<decltype(&FontAwesome::UnicodeToUtf8), std::string (*)(unsigned int)>);

#if !defined(MENU_WINDOW)
#error SFSE-MCP must preserve the SKSE-MCP MENU_WINDOW macro.
#endif

ButtonFunction volatile ConsumerButton = &ImGuiMCP::Button;

[[maybe_unused]] MCP::Event *CompileDefaultEventPriority(EventCallback callback) {
    return Framework::AddEvent(callback);
}

SFSEMCP::detail::ProviderBinding* OtherBindingAddress();

int main() {
    return ConsumerButton == nullptr || OtherBindingAddress() != &SFSEMCP::detail::Binding() ? 1 : 0;
}
