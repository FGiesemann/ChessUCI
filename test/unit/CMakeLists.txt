add_executable(chessuci_unittests
    src/engine_handler_callback_test.cpp
    src/engine_handler_parsing_test.cpp
    src/gui_handler_callback_test.cpp
    src/gui_handler_parsing_test.cpp
    src/uci_move_matcher_test.cpp
    src/uci_move_parser_test.cpp

    helper/EngineProcessMock.cpp
)
target_include_directories(chessuci_unittests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(chessuci_unittests PRIVATE
    TEST_BINARY_DIR="${CMAKE_BINARY_DIR}/test/processes/test_binaries"
)
add_dependencies(chessuci_unittests
    test_echo
)

if (WIN32)
    target_sources(chessuci_unittests 
        PRIVATE
            src/unicode_conversion_win_test.cpp
    )
endif()

add_compiler_warnings(chessuci_unittests)

target_link_libraries(chessuci_unittests
    PRIVATE
        ChessUCI
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(chessuci_unittests
    PROPERTIES TIMEOUT 20
)
