#include "chessuci/process_factory.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string>
#include <thread>

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

TEST_CASE("ProcessTests.Process can be started", "[process][basic]") {
    auto process = chessuci::ProcessFactory::create_local();
    REQUIRE(process);

    auto binary = get_test_binary_path("test_echo");
    REQUIRE(process->start({binary, {"hello", "world"}}));

    REQUIRE(process->is_running());
    REQUIRE(process->pid() > 0);
}

TEST_CASE("ProcessTests.Process exits immediately", "[process][exit]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_immediate_exit");
    REQUIRE(process->start({binary, {"42"}}));

    auto exit_code = process->wait_for_exit(5000);
    REQUIRE(exit_code.has_value());
    REQUIRE(*exit_code == 42);

    REQUIRE_FALSE(process->is_running());
}

TEST_CASE("ProcessTests.Process start failure is detected", "[process][error]") {
    auto process = chessuci::ProcessFactory::create_local();

    REQUIRE_FALSE(process->start({"/nonexistent/binary"}));

    std::string error = process->last_error();
    REQUIRE_FALSE(error.empty());
}

TEST_CASE("ProcessTests.Can write to process stdin", "[process][io]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_line_echo");
    REQUIRE(process->start({binary}));

    REQUIRE(process->write_line("hello"));

    std::string response;
    REQUIRE(process->read_line(response));
    REQUIRE(response == "hello");

    process->write_line("quit");
    process->wait_for_exit(1000);
}

TEST_CASE("ProcessTests.Can read multiple lines", "[process][io]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_line_echo");
    REQUIRE(process->start({binary}));

    REQUIRE(process->write_line("line1"));
    REQUIRE(process->write_line("line2"));
    REQUIRE(process->write_line("line3"));

    std::string line;
    REQUIRE(process->read_line(line));
    REQUIRE(line == "line1");

    REQUIRE(process->read_line(line));
    REQUIRE(line == "line2");

    REQUIRE(process->read_line(line));
    REQUIRE(line == "line3");

    process->write_line("quit");
    process->wait_for_exit(1000);
}

TEST_CASE("ProcessTests.Handle large output", "[process][io][stress]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_output_flood");
    REQUIRE(process->start({binary, {"1000"}}));

    int lines_read = 0;
    std::string line;

    while (process->read_line(line)) {
        ++lines_read;
        REQUIRE(line.find("Line") != std::string::npos);
    }

    REQUIRE(lines_read == 1000);
    process->wait_for_exit(100);
    REQUIRE_FALSE(process->is_running());
}

TEST_CASE("ProcessTests.Graceful termination with quit", "[process][terminate]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_line_echo");
    REQUIRE(process->start({binary}));
    REQUIRE(process->is_running());

    REQUIRE(process->terminate(5000));
    REQUIRE_FALSE(process->is_running());
}

TEST_CASE("ProcessTests.Force kill hanging process", "[process][kill]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_hang");
    REQUIRE(process->start({binary}));
    REQUIRE(process->is_running());

    // should timeout
    REQUIRE_FALSE(process->terminate(100)); // 100ms timeout

    process->kill();
    REQUIRE_FALSE(process->is_running());
}

TEST_CASE("ProcessTests.Detect crashed process", "[process][crash]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_crash");
    REQUIRE(process->start({binary, {"500"}})); // Crash after 500ms

    REQUIRE(process->is_running());

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE_FALSE(process->is_running());

    // exit code != 0 -> Crash
    auto exit_code = process->wait_for_exit(0);
    REQUIRE(exit_code.has_value());
    REQUIRE(*exit_code != 0);
}

TEST_CASE("ProcessTests.Working directory is set correctly", "[process][workdir]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_working_dir");

    auto temp_dir = fs::temp_directory_path();
    REQUIRE(process->start({binary, {}, temp_dir.string()}));

    std::string output;
    REQUIRE(process->read_line(output));

    fs::path result_path(output);
    REQUIRE(fs::equivalent(result_path, temp_dir));

    process->wait_for_exit(1000);
}

#ifdef __unix__
TEST_CASE("ProcessTests.Handle zombie process (Unix)", "[process][unix][zombie]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_zombie");
    REQUIRE(process->start({binary}));
    REQUIRE(process->is_running());

    REQUIRE_FALSE(process->terminate(1000));

    process->kill();
    REQUIRE_FALSE(process->is_running());
}
#endif

TEST_CASE("ProcessTests.Wait with timeout", "[process][timeout]") {
    auto process = chessuci::ProcessFactory::create_local();

    auto binary = get_test_binary_path("test_hang");
    REQUIRE(process->start({binary}));

    // should timeout
    auto result = process->wait_for_exit(100);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(process->is_running());

    process->kill();
}

TEST_CASE("ProcessTests.can_read() detects available data", "[process][io]") {
    auto process = chessuci::ProcessFactory::create_local();
    auto binary = get_test_binary_path("test_line_echo");
    REQUIRE(process->start({binary}));

    REQUIRE_FALSE(process->can_read());

    REQUIRE(process->write_line("test"));

    // wait for data
    auto start = std::chrono::steady_clock::now();
    while (!process->can_read()) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > std::chrono::seconds(5)) {
            FAIL("Timeout waiting for output");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::string output;
    REQUIRE(process->read_line(output));
    REQUIRE(output == "test");

    process->terminate();
}
