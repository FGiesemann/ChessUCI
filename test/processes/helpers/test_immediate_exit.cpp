#include <cstdlib>

auto main(int argc, char *argv[]) -> int {
    if (argc > 1) {
        return std::atoi(argv[1]);
    }
    return 0;
}
