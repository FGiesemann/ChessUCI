add_executable(chessuci_unittests
    src/engine_handler_parsing_test.cpp
    src/gui_handler_parsing_test.cpp
    src/uci_move_matcher_test.cpp
    src/uci_move_parser_test.cpp
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
catch_discover_tests(chessuci_unittests)
