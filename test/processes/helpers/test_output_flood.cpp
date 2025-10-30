#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
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
