#include <filesystem>
#include <iostream>

auto main() -> int {
    try {
        auto cwd = std::filesystem::current_path();
        std::cout << cwd.string() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
