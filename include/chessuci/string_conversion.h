/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_STRING_CONVERSION_H
#define CHESSUCI_STRING_CONVERSION_H

#include <optional>
#include <sstream>
#include <string_view>

namespace chessuci {

template<typename T>
requires std::integral<T>
auto str_to_inttype(std::string_view str) -> std::optional<T> {
    if (str.empty() || str.find_first_not_of(" \t\n\r") == std::string::npos) {
        return std::nullopt;
    }

    T value{};
    const char *first = str.data();
    const char *last = &*str.end();
    auto [ptr, ec] = std::from_chars(first, last, value);

    if (ec == std::errc()) {
        while (ptr != last && (*ptr == ' ' || *ptr == '\t')) {
            std::advance(ptr, 1);
        }
        if (ptr == last) {
            return value;
        }
    }
    return std::nullopt;
}

} // namespace chessuci

#endif
