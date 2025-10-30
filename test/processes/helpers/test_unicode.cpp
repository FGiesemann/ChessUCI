#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc > 1) {
        std::cout << argv[1] << std::endl;
    } else {
        std::cout << "Hello 世界 🌍 Schach ♔♕♖♗♘♙" << std::endl;
    }
    return 0;
}
