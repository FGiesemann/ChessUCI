add_executable(chessuci_tests
    src/uci_move_matcher_test.cpp
    src/uci_move_parser_test.cpp
)
add_compiler_warnings(chessuci_tests)

target_link_libraries(chessuci_tests
  PRIVATE 
  ChessUCI
  Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(chessuci_tests)
