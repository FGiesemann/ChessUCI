#include <iostream>
#include <string>

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "quit") {
            break;
        }
        std::cout << line << std::endl;
        std::cout.flush();
    }
    return 0;
}
