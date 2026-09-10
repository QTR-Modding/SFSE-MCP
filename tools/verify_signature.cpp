#include <CLibUtilsQTR/Signing/CommandLine.hpp>
#include <SFSEMCP/detail/SigningKey.hpp>

int wmain(int argc, wchar_t** argv) {
    return clib_utilsQTR::Signing::VerifyFileCommandLine(argc, argv, &SFSEMCP::detail::ReleaseSigningKey);
}
