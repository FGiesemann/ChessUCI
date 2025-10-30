/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/engine_process_win.h"
#include <catch2/catch_test_macros.hpp>

using namespace chessuci;

TEST_CASE("Unicode.utf8 to wstr", "[unicode]") {
    std::string str = "Hello, 🌍♔";
    std::wstring wstr = EngineProcessWin::utf8_to_wide(str);

    REQUIRE(wstr == L"Hello, \xD83C\xDF0D\x2654");
}

TEST_CASE("Unicode.wstr to utf8", "[unicode]") {
    std::wstring wstr = L"Hello, \xD83C\xDF0D\x2654";
    std::string str = EngineProcessWin::wide_to_utf8(wstr);

    REQUIRE(str == "Hello, 🌍♔");
}
