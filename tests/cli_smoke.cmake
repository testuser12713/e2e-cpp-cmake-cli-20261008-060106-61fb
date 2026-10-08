# Office-independent smoke test driven by CTest: run the csvstats executable
# with no arguments and require the usage line on STDERR.
#
# Invoked as:  cmake -D CSVSTATS_BIN=<path> -P cli_smoke.cmake
# (execute_process already separates stdout and stderr, so the assertion is
# about the stderr stream specifically.)

if(NOT DEFINED CSVSTATS_BIN)
    message(FATAL_ERROR "CSVSTATS_BIN is not set")
endif()

execute_process(
    COMMAND "${CSVSTATS_BIN}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
)

if(NOT err MATCHES "Usage: csvstats \\[OPTIONS\\] <file>")
    message(FATAL_ERROR
        "cli_smoke: usage line not found on stderr (exit code ${result}).\n"
        "stderr was: ${err}")
endif()
