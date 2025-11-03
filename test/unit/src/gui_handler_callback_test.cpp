/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/gui_handler.h"
#include <catch2/catch_test_macros.hpp>

#include "helper/EngineProcessMock.h"

#include <future>

using namespace chessuci;

namespace fs = std::filesystem;

inline auto get_test_binary_path(const std::string &name) -> std::string {
    fs::path binary_dir(TEST_BINARY_DIR);
#ifdef _WIN32
    fs::path binary = binary_dir / (name + ".exe");
#else
    fs::path binary = binary_dir / name;
#endif
    return binary.string();
}

TEST_CASE("GuiHandler.Callback.No Callbacks", "[gui_handler]") {
    UCIGuiHandler handler{};
    auto binary = get_test_binary_path("test_echo");

    CHECK_FALSE(handler.is_running());
    handler.start({binary});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CHECK(handler.is_running());
    CHECK(handler.process().is_running());
    handler.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK_FALSE(handler.is_running());
    CHECK_FALSE(handler.process().is_running());
}

TEST_CASE("EngineProcessMock", "[engine_process_mock]") {
    test::EngineProcessMock mock;

    mock.when_receives("test", [](const std::string &) -> std::vector<std::string> { return {"response1", "response2"}; });
    mock.when_receives("stop", [](const std::string &) -> std::vector<std::string> { return {"stop requested"}; });

    mock.write_line("test");

    std::string line;
    REQUIRE(mock.read_line(line));
    CHECK(line == "response1");
    REQUIRE(mock.read_line(line));
    CHECK(line == "response2");
    CHECK_FALSE(mock.can_read());

    mock.write_line("stop");
    REQUIRE(mock.read_line(line));
    CHECK(line == "stop requested");
    CHECK_FALSE(mock.can_read());
}

TEST_CASE("GuiHandler.Callback.Quit", "[gui_handler]") {
    std::promise<void> quit_done;
    auto quit_future = quit_done.get_future();
    auto mock_engine = std::make_unique<test::EngineProcessMock>();
    mock_engine->when_receives("quit", [&quit_done](const std::string &) -> std::vector<std::string> {
        quit_done.set_value();
        return {};
    });

    UCIGuiHandler handler{std::move(mock_engine)};

    handler.start({});
    handler.send_quit();
    REQUIRE(quit_future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    handler.stop();
}

TEST_CASE("GuiHandler.Callback.Isready", "[gui_handler]") {
    auto mock_engine = std::make_unique<test::EngineProcessMock>();
    mock_engine->when_receives("isready", [](const std::string &) -> std::vector<std::string> { return {"readyok"}; });

    UCIGuiHandler handler{std::move(mock_engine)};
    std::promise<void> isready_done;
    auto isready_future = isready_done.get_future();
    handler.on_readyok([&isready_done]() -> void { isready_done.set_value(); });

    REQUIRE(handler.start({}));
    REQUIRE(handler.send_isready());
    REQUIRE(isready_future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    handler.stop();
}

TEST_CASE("GuiHandler.Callback.UCI", "[gui_handler]") {
    auto mock_engine = std::make_unique<test::EngineProcessMock>();
    mock_engine->when_receives("uci", [](const std::string &) -> std::vector<std::string> { return {"id name test_engine", "id author test_author", "uciok"}; });

    UCIGuiHandler handler{std::move(mock_engine)};

    std::promise<std::string> id_name_done;
    auto id_name_future = id_name_done.get_future();
    handler.on_id_name([&id_name_done](const std::string &name) -> void { id_name_done.set_value(name); });
    std::promise<std::string> id_author_done;
    auto id_author_future = id_author_done.get_future();
    handler.on_id_author([&id_author_done](const std::string &author) -> void { id_author_done.set_value(author); });
    std::promise<void> uciok_done;
    auto uciok_future = uciok_done.get_future();
    handler.on_uciok([&uciok_done]() -> void { uciok_done.set_value(); });

    REQUIRE(handler.start({}));
    REQUIRE(handler.send_uci());
    REQUIRE(id_name_future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    CHECK(id_name_future.get() == "test_engine");
    REQUIRE(id_author_future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    CHECK(id_author_future.get() == "test_author");
    REQUIRE(uciok_future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);

    handler.stop();
}
