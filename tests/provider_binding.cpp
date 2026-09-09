#include <SFSEMCP/SFSEMenuFramework.hpp>

#include <array>
#include <thread>

namespace {
using namespace SFSEMCP::detail;
decltype(&CreateFileW) originalCreateFile = &CreateFileW;
const wchar_t* baitPath{};
struct TeardownLookup {
    bool armed{};
    ~TeardownLookup() {
        if (armed && !SFSEMenuFramework::IsHotkeyEnabled()) TerminateProcess(GetCurrentProcess(), 28);
    }
} teardownLookup;

decltype(&GetModuleHandleW) originalModule = &GetModuleHandleW;
decltype(&GetProcAddress) originalFunction = &GetProcAddress;
unsigned fakeCalls{};
decltype(&GetModuleHandleExW) originalModuleReference = &GetModuleHandleExW;
decltype(&VirtualQuery) originalQuery = &VirtualQuery;
std::atomic<unsigned> queryCalls{};

HWND WINAPI ConsoleForTest() { return reinterpret_cast<HWND>(1); }
BOOL WINAPI FailEnumeration(HANDLE, HMODULE*, DWORD, LPDWORD) { return FALSE; }
HANDLE WINAPI FailOpen(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE) {
    return INVALID_HANDLE_VALUE;
}
LPVOID WINAPI FailMap(HANDLE, DWORD, DWORD, DWORD, SIZE_T) { return nullptr; }
BOOL WINAPI FailPin(DWORD flags, LPCWSTR name, HMODULE* module) {
    return (flags & GET_MODULE_HANDLE_EX_FLAG_PIN) ? FALSE : originalModuleReference(flags, name, module);
}
SIZE_T WINAPI CountQuery(LPCVOID address, PMEMORY_BASIC_INFORMATION information, SIZE_T length) {
    ++queryCalls;
    return originalQuery(address, information, length);
}

bool FakeButton(const char*, ImGuiMCP::ImVec2) { ++fakeCalls; return false; }
HMODULE WINAPI AliasModule(LPCWSTR name) {
    return name && _wcsicmp(name, L"SFSEMenuFramework.dll") == 0 ?
        reinterpret_cast<HMODULE>(0x12340000) : originalModule(name);
}
FARPROC WINAPI AliasFunction(HMODULE module, LPCSTR name) {
    return module == reinterpret_cast<HMODULE>(0x12340000) &&
        reinterpret_cast<std::uintptr_t>(name) > 65535 && std::strcmp(name, "igButton") == 0 ?
        reinterpret_cast<FARPROC>(&FakeButton) : originalFunction(module, name);
}
HANDLE WINAPI BaitFile(LPCWSTR, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES attributes,
                      DWORD disposition, DWORD flags, HANDLE templateFile) {
    return originalCreateFile(baitPath, access, share, attributes, disposition, flags, templateFile);
}

// This fixture changes only its own import slots. It is never a game plugin.
bool PatchImport(const char* name, FARPROC replacement) {
    auto* base = reinterpret_cast<std::byte*>(GetModuleHandleW(nullptr));
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    auto* imports = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base +
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (; imports->Name; ++imports) {
        auto* names = reinterpret_cast<IMAGE_THUNK_DATA64*>(base + imports->OriginalFirstThunk);
        auto* slots = reinterpret_cast<IMAGE_THUNK_DATA64*>(base + imports->FirstThunk);
        for (; names->u1.AddressOfData; ++names, ++slots) {
            if (IMAGE_SNAP_BY_ORDINAL64(names->u1.Ordinal)) continue;
            auto* entry = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + names->u1.AddressOfData);
            if (std::strcmp(entry->Name, name) != 0) continue;
            DWORD old{};
            if (!VirtualProtect(slots, sizeof(*slots), PAGE_READWRITE, &old)) return false;
            slots->u1.Function = reinterpret_cast<ULONGLONG>(replacement);
            DWORD ignored{};
            return VirtualProtect(slots, sizeof(*slots), old, &ignored) != FALSE;
        }
    }
    return false;
}

bool WriteMemory(void* address, const void* bytes, std::size_t size) {
    DWORD old{}, ignored{};
    if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old)) return false;
    std::memcpy(address, bytes, size);
    FlushInstructionCache(GetCurrentProcess(), address, size);
    return VirtualProtect(address, size, old, &ignored) != FALSE;
}

int CheckExports(HMODULE module, bool rejectThroughPublicAPI = false) {
    VerifiedProvider provider;
    if (provider.Bind(module, ReleaseSigningKey) != BindingError::None) return 20;
    BindingError error{};
    if (provider.Resolve("NotAnExport", error) || error != BindingError::None) return 21;
    if (provider.Bind(module, ReleaseSigningKey) != BindingError::Inspection) return 22;
    auto* base = reinterpret_cast<std::byte*>(module);
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    const auto directory = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    auto* exports = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(base + directory.VirtualAddress);
    auto* names = reinterpret_cast<DWORD*>(base + exports->AddressOfNames);
    auto* ordinals = reinterpret_cast<WORD*>(base + exports->AddressOfNameOrdinals);
    auto* functions = reinterpret_cast<DWORD*>(base + exports->AddressOfFunctions);
    for (DWORD i = 0; i < exports->NumberOfNames; ++i) {
        if (std::strcmp(reinterpret_cast<char*>(base + names[i]), "igButton") != 0) continue;
        auto* slot = functions + ordinals[i];
        const DWORD saved = *slot;
        // Out-of-image, header/data, forwarded export, and different in-code RVA.
        for (DWORD changed : std::array<DWORD, 4>{0xFFFFFFFFu, 1u, directory.VirtualAddress, saved + 1}) {
            if (!WriteMemory(slot, &changed, sizeof(changed))) return 23;
            if (rejectThroughPublicAPI) {
                ImGuiMCP::Button("MCP dispatch", {42, 24});
                return 35;  // Must terminate with the export diagnostic instead.
            }
            const auto result = provider.Resolve("igButton", error);
            if (!WriteMemory(slot, &saved, sizeof(saved))) return 24;
            if (result || error != BindingError::Export) return 25;
        }
        return provider.Resolve("igButton", error) && error == BindingError::None ? 0 : 26;
    }
    return 27;
}
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 3) return 2;
    const std::wstring_view mode = argv[1];
    teardownLookup.armed = mode == L"teardown";
    if (mode == L"retry" && SFSEMenuFramework::IsHotkeyEnabled()) return 3;
    HMODULE module = LoadLibraryW(argv[2]);
    if (!module) return 4;
    if (mode.starts_with(L"inspection_") || mode == L"reject_export") {
        // Select stderr diagnostics without opening a dialog or creating a console.
        if (!PatchImport("GetConsoleWindow", reinterpret_cast<FARPROC>(&ConsoleForTest))) return 29;
        std::setvbuf(stderr, nullptr, _IONBF, 0);
        if (mode == L"reject_export") {
            if (GetMenuFrameworkModule() != module) return 30;
            return CheckExports(module, true);
        }
        const char* importName{};
        FARPROC replacement{};
        if (mode == L"inspection_enum") { importName = "K32EnumProcessModules"; replacement = reinterpret_cast<FARPROC>(&FailEnumeration); }
        if (mode == L"inspection_file") { importName = "CreateFileW"; replacement = reinterpret_cast<FARPROC>(&FailOpen); }
        if (mode == L"inspection_map") { importName = "MapViewOfFile"; replacement = reinterpret_cast<FARPROC>(&FailMap); }
        if (mode == L"inspection_pin") { importName = "GetModuleHandleExW"; replacement = reinterpret_cast<FARPROC>(&FailPin); }
        if (!importName || !PatchImport(importName, replacement)) return 31;
        ImGuiMCP::Button("MCP dispatch", {42, 24});
        return 32;  // Neither a null call nor silent failure is acceptable.
    }
    if (mode == L"cache") {
        if (!PatchImport("VirtualQuery", reinterpret_cast<FARPROC>(&CountQuery))) return 33;
        std::string name = "igButton";
        const auto button = VerifiedFunction(name.c_str());
        if (!button || queryCalls == 0) return 34;
        name.assign("NoButton");
        if (VerifiedFunction(name.c_str())) return 36;
        queryCalls = 0;
        for (unsigned i = 0; i < 10000; ++i) {
            if (!ImGuiMCP::Button("MCP dispatch", {42, 24}) ||
                VerifiedFunction("igButton") != button || VerifiedFunction("NoButton")) return 37;
        }
        if (queryCalls != 0) return 38;
    }
    if (mode == L"signature") {
        VerifiedProvider provider;
        return provider.Bind(module, ReleaseSigningKey) == BindingError::Signature ? 0 : 5;
    }
    if (mode == L"fatal") {
        GetMenuFrameworkModule();
        return 6;
    }
    if (mode == L"exports") return CheckExports(module);
    if (mode == L"code") {
        auto* target = reinterpret_cast<std::byte*>(GetProcAddress(module, "igButton"));
        if (!target) return 7;
        const auto original = *target;
        const auto changed = original ^ std::byte{1};
        if (!WriteMemory(target, &changed, 1)) return 8;
        VerifiedProvider provider;
        const auto result = provider.Bind(module, ReleaseSigningKey);
        if (!WriteMemory(target, &original, 1)) return 9;
        return result == BindingError::Image ? 0 : 10;
    }
    if (mode == L"bait") {
        if (argc != 4) return 11;
        baitPath = argv[3];
        if (!PatchImport("CreateFileW", reinterpret_cast<FARPROC>(&BaitFile))) return 12;
        VerifiedProvider provider;
        return provider.Bind(module, ReleaseSigningKey) == BindingError::Image ? 0 : 13;
    }
    if (mode == L"alias") {
        // Patch GetModuleHandle last: the patch helper uses it to find this EXE.
        if (!PatchImport("GetProcAddress", reinterpret_cast<FARPROC>(&AliasFunction)) ||
            !PatchImport("GetModuleHandleW", reinterpret_cast<FARPROC>(&AliasModule))) return 14;
        using Button = bool (*)(const char*, ImGuiMCP::ImVec2);
        const auto oldLookup = reinterpret_cast<Button>(GetProcAddress(GetModuleHandleW(L"SFSEMenuFramework.dll"), "igButton"));
        if (oldLookup("MCP dispatch", {42, 24}) || fakeCalls != 1) return 15;
        fakeCalls = 0;
    }
    if (mode == L"concurrent") {
        std::atomic<unsigned> passed{};
        std::array<std::thread, 16> threads;
        for (auto& thread : threads) thread = std::thread([&] {
            if (GetMenuFrameworkModule() == module && SFSEMenuFramework::IsHotkeyEnabled() &&
                ImGuiMCP::Button("MCP dispatch", {42, 24})) ++passed;
        });
        for (auto& thread : threads) thread.join();
        if (passed != threads.size()) return 16;
    }
    if (GetMenuFrameworkModule() != module || !SFSEMenuFramework::IsHotkeyEnabled() ||
        !ImGuiMCP::Button("MCP dispatch", {42, 24}) || fakeCalls) return 17;
    std::puts("Verified provider called; fake provider was not called.");
    return 0;
}
