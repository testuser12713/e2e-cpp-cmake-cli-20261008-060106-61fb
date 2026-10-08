# Unit tests for the csvstats_core library plus one smoke test that runs the
# built csvstats executable. All five cases run under CTest.

add_executable(test_csv test_csv.cpp)
target_link_libraries(test_csv PRIVATE csvstats_core)
add_test(NAME csv COMMAND test_csv)

add_executable(test_stats test_stats.cpp)
target_link_libraries(test_stats PRIVATE csvstats_core)
add_test(NAME stats COMMAND test_stats)

add_executable(test_report test_report.cpp)
target_link_libraries(test_report PRIVATE csvstats_core)
add_test(NAME report COMMAND test_report)

add_executable(test_cli test_cli.cpp)
target_link_libraries(test_cli PRIVATE csvstats_core)
add_test(NAME cli COMMAND test_cli)

# Smoke test: run the real csvstats binary (no arguments) and require the
# usage line to appear on stderr. The wrapper script separates the streams,
# so a usage line that leaks to stdout would not satisfy it.
add_test(
    NAME cli_smoke
    COMMAND ${CMAKE_COMMAND}
        -D CSVSTATS_BIN=$<TARGET_FILE:csvstats>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cli_smoke.cmake
)
