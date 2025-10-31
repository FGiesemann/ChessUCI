#include <iostream>
#include <string>

auto main(int argc, char *argv[]) -> int {
    int lines = 1000;
    if (argc > 1) {
        lines = std::atoi(argv[1]);
    }

    for (int i = 0; i < lines; ++i) {
        std::cout << "Line " << i << ": Lorem ipsum dolor sit amet, "
                  << "consectetur adipiscing elit." << std::endl;
    }

    return 0;
}
