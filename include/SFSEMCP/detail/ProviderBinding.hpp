#pragma once

#include "VerifiedProvider.hpp"
#if defined(SFSEMCP_SIGNING_KEY_HEADER)
// Explicit compile-time trust selection, used by isolated tests and rebuilds.
// A distributed client has one fixed key; there is no runtime bypass setting.
#include SFSEMCP_SIGNING_KEY_HEADER
#else
#include "SigningKey.hpp"
#endif

#include <cstdio>
#include <utility>
#include <atomic>
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
        std::lock_guard lock(mutex_);
        if (provider_) return provider_.get();
        BindingError error{};
        auto module = FindProvider(error);
        if (error == BindingError::Missing || error == BindingError::Inspection) return nullptr;
        if (error != BindingError::None) RejectProvider(error);
        auto candidate = std::make_unique<VerifiedProvider>();
        error = candidate->Bind(module, ReleaseSigningKey);
        if (error == BindingError::Inspection) return nullptr;
        if (error != BindingError::None) RejectProvider(error);
        // Accepted framework code must remain resident through client teardown.
        HMODULE pinned{};
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                reinterpret_cast<LPCWSTR>(module), &pinned) || pinned != module) return nullptr;
        provider_ = std::move(candidate);
        return provider_.get();
    }

private:
    std::mutex mutex_;
    std::unique_ptr<VerifiedProvider> provider_;
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
    auto* provider = Binding().Get();
    if (!provider) return nullptr;
    BindingError error{};
    auto function = provider->Resolve(name, error);
    if (error != BindingError::None) RejectProvider(error);
    return function;
}

// Framework registration can precede provider loading. Do not permanently cache
// a null lookup; successful bindings remain cheap, thread-safe function pointers.
template<class T>
class OptionalFunction {
public:
    explicit OptionalFunction(const char* name) : name_(name) {}
    explicit operator bool() const { return Get() != nullptr; }
    operator T() const { return Get(); }

    template<class... Args>
    decltype(auto) operator()(Args&&... args) const {
        return Get()(std::forward<Args>(args)...);
    }

private:
    T Get() const {
        auto value = value_.load(std::memory_order_acquire);
        if (!value) {
            value = reinterpret_cast<T>(VerifiedFunction(name_));
            if (value) value_.store(value, std::memory_order_release);
        }
        return value;
    }
    const char* name_;
    mutable std::atomic<T> value_{};
};

}
