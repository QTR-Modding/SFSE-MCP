#include <SFSEMCP/detail/SigningKey.hpp>

#include <cstdio>
#include <cwchar>

int wmain(int argc, wchar_t** argv) {
    if (argc != 2 && argc != 3) return 2;
    if (argc == 3 && std::wcslen(argv[2]) != 64) return 2;
    const auto hex = [](wchar_t value) -> int {
        if (value >= L'0' && value <= L'9') return value - L'0';
        if (value >= L'a' && value <= L'f') return value - L'a' + 10;
        if (value >= L'A' && value <= L'F') return value - L'A' + 10;
        return -1;
    };
    auto key = SFSEMCP::detail::ReleaseSigningKey;
    for (std::size_t index = 0; argc == 3 && index < key.size(); ++index) {
        const auto high = hex(argv[2][index * 2]);
        const auto low = hex(argv[2][index * 2 + 1]);
        if (high < 0 || low < 0) return 2;
        key[index] = static_cast<BYTE>((high << 4) | low);
    }
    SFSEMCP::detail::SignedFile file;
    if (!file.Open(argv[1])) {
        std::puts("Cannot read candidate.");
        return 3;
    }
    if (!SFSEMCP::detail::VerifySignature(file, key)) {
        std::puts("Signature rejected.");
        return 4;
    }
    std::puts("Verified signing key and file digest.");
    return 0;
}
