#pragma once

#include "VerifiedProvider.hpp"
#include <CLibUtilsQTR/Signing.hpp>
#if defined(SFSEMCP_SIGNING_KEY_HEADER)
// Explicit compile-time trust selection, used by isolated tests and rebuilds.
// A distributed client has one fixed key; there is no runtime bypass setting.
#include SFSEMCP_SIGNING_KEY_HEADER
#else
#include "SigningKey.hpp"
#endif

#include <cstdio>
#include <cstdlib>

namespace SFSEMCP::detail {

[[noreturn]] inline void RejectProvider(BindingError error) {
    const wchar_t* reason = L"The loaded framework could not be inspected.";
    if (error == BindingError::Signature) reason = L"The framework signature or signing key is not accepted.";
    if (error == BindingError::Image) reason = L"The loaded framework does not match its signed file, or multiple copies are loaded.";
    if (error == BindingError::Export) reason = L"A framework function is forwarded, modified, or outside the verified executable code.";
    std::wstring message = L"SFSE Menu Framework verification failed.\n\n";
    message += reason;
    message += L"\n\nInstall the matching official framework and client versions, and remove conflicting replacements. "
               L"The process will stop without calling the unverified provider.";
    OutputDebugStringW(message.c_str());
    std::fwprintf(stderr, L"%ls\n", message.c_str());
    // Automated console fixtures receive the same diagnostic without a dialog.
    if (!GetConsoleWindow()) MessageBoxW(nullptr, message.c_str(), L"SFSE Menu Framework", MB_OK | MB_ICONERROR);
    TerminateProcess(GetCurrentProcess(), 0x53464D46);
    std::abort();
}

class ProviderBinding {
public:
    VerifiedProvider* Get() {
        BindingError error{};
        auto* provider = binding_.Get(error);
        Check(error);
        return provider;
    }

    FARPROC Resolve(std::string_view name) {
        BindingError error{};
        auto function = binding_.Resolve(name, error);
        Check(error);
        return function;
    }

private:
    static void Check(BindingError error) {
        if (error != BindingError::None && error != BindingError::Missing) RejectProvider(error);
    }
    clib_utilsQTR::Signing::ProviderBinding binding_{L"SFSEMenuFramework.dll", ReleaseSigningKey};
};

inline ProviderBinding& Binding() {
    // Client registration owners can unregister during global destruction.
    // Retain one binding per client for process lifetime, like the pinned DLL;
    // destroying it earlier would leave those final lookups using dead state.
    static auto* binding = new ProviderBinding;
    return *binding;
}

inline HMODULE VerifiedModule() {
    auto* provider = Binding().Get();
    return provider ? provider->Module() : nullptr;
}

inline FARPROC VerifiedFunction(const char* name) {
    return Binding().Resolve(name);
}

template<class T>
using OptionalFunction = clib_utilsQTR::Signing::OptionalFunction<T, VerifiedFunction>;

}
