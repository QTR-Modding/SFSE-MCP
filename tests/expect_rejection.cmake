execute_process(
    COMMAND "${EXECUTABLE}" "${MODE}" "${HOST_DLL}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    TIMEOUT 10
)
math(EXPR expected "0x53464D46")
if(MODE STREQUAL "reject_export")
    set(reason "A framework function is forwarded, modified")
else()
    set(reason "The loaded framework could not be inspected")
endif()
if(NOT result STREQUAL "${expected}" OR NOT error MATCHES "${reason}")
    message(FATAL_ERROR "Expected controlled rejection, got ${result}: ${output}${error}")
endif()
