/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/protocol.h"

#include <sstream>

namespace chessuci {

auto Option::to_uci_string() const -> std::string {
    std::ostringstream oss;
    oss << "option ";
    oss << "name " << name;
    oss << "type " << type_to_string();
    if (default_value.has_value()) {
        oss << "default " << *default_value;
    }
    if (min.has_value()) {
        oss << "min " << *min;
    }
    if (max.has_value()) {
        oss << "max " << *max;
    }
    for (const auto &value : combo_values) {
        oss << "var " << value;
    }
    return oss.str();
}

auto Option::type_to_string() const -> std::string {
    switch (type) {
    case Type::Button:
        return "button";
    case Type::Check:
        return "check";
    case Type::Combo:
        return "combo";
    case Type::Spin:
        return "spin";
    case Type::String:
        return "string";
    default:
        return "<unknown>";
    }
}

auto Option::type_from_string(const std::string &str) -> Type {
    if (str == "button") {
        return Type::Button;
    } else if (str == "check") {
        return Type::Check;
    } else if (str == "combo") {
        return Type::Combo;
    } else if (str == "spin") {
        return Type::Spin;
    } else if (str == "string") {
        return Type::String;
    } else {
        throw UCIError("unknown option type: " + str);
    }
}

} // namespace chessuci
