#include "json.h"

using namespace std;

namespace json {

namespace load{

using Number = std::variant<int, double>;

Node LoadNode(std::istream& input);

Number LoadNumber(std::istream& input) {

    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }

    if (input.peek() == '0') {
        read_char();
    } else {
        read_digits();
    }

    bool is_int = true;
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            try {
                return std::stoi(parsed_num);
            } catch (...) {}
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error"s);
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error"s);
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

bool LoadBool(std::istream& input) {
    std::string parsed;
    while (std::isalpha(input.peek())) {
        parsed += static_cast<char>(input.get());
    }
    if (parsed == "true"s) {
        return true;
    } else if (parsed == "false"s) {
        return false;
    } else {
        throw ParsingError("Failed to parse boolean value"s);
    }
}

Node LoadNull(std::istream& input) {
    std::string parsed;
    while (std::isalpha(input.peek())) {
        parsed += static_cast<char>(input.get());
    }
    if (parsed != "null"s) {
        throw ParsingError("Failed to parse null value"s);
    }
    return Node{nullptr};
}

Array LoadArray(std::istream& input) {
    Array result;
    char ch;
    if (!(input >> ch) || ch != '[') {
        throw ParsingError("Array must start with '['"s);
    }
    for (char c = input.peek(); c != ']'; c = input.peek()) {
        result.push_back(LoadNode(input));
        input >> std::ws; 
        if (input.peek() == ',') {
            input.get();
        }
    }
    input.get(); 
    return result;
}

Dict LoadDict(std::istream& input) {
    Dict result;
    char ch;
    if (!(input >> ch) || ch != '{') {
        throw ParsingError("Dict must start with '{'"s);
    }
    for (char c = input.peek(); c != '}'; c = input.peek()) {
        input >> std::ws; 
        input >> c;
        std::string key = LoadString(input); 
        input >> std::ws;
        if (input.get() != ':') {
            throw ParsingError("Expected ':' after key in dict"s);
        }
        input >> std::ws;
        result[std::move(key)] = LoadNode(input);
        input >> std::ws;
        if (input.peek() == ',') {
            input.get(); 
        }
    }
    input.get(); 
    return result;
}

Node LoadNode(std::istream& input) {
    input >> std::ws; 
    char ch = input.peek();
    if (ch == '[') {
        return LoadArray(input);
    } else if (ch == '{') {
        return LoadDict(input);
    } else if (ch == '"') {
        input >> ch;
        return Node(LoadString(input));
    } else if (std::isdigit(ch) || ch == '-' || ch == '.') {
        auto number = LoadNumber(input);
        if (std::holds_alternative<int>(number)) {
            return Node(std::get<int>(number));
        } else {
            return Node(std::get<double>(number));
        }
    } else if (ch == 't' || ch == 'f') {
        return Node(LoadBool(input));
    } else if (ch == 'n') {
        return LoadNull(input);
    } else {
        throw ParsingError("Unexpected character in JSON"s);
    }
}

}  // load
    
Node::Node() : value_(nullptr) {}
Node::Node(std::nullptr_t) : value_(nullptr) {}
Node::Node(const Array& array) : value_(array) {}
Node::Node(const Dict& dict) : value_(dict) {}
Node::Node(bool b) : value_(b) {}
Node::Node(int i) : value_(i) {}
Node::Node(double d) : value_(d) {}
Node::Node(const std::string& str) : value_(str) {}

const Node::Value& Node::GetValue() const {
        return value_; 
    }
    
bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_); 
}
bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_); 
}
bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_); 
}
bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_); 
}
bool Node::IsInt() const {
    return std::holds_alternative<int>(value_); 
}
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_); 
}
bool Node::IsDouble() const {
    return IsPureDouble() || IsInt(); 
}
bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_); 
}
    
const Array& Node::AsArray() const {
    if (!IsArray()) throw std::logic_error("No vector"s);
    return std::get<Array>(value_);
}
const Dict& Node::AsMap() const {
    if (!IsMap()) throw std::logic_error("No map"s);
    return std::get<Dict>(value_);
}
bool Node::AsBool() const {
    if (!IsBool()) throw std::logic_error("No bool"s);
    return std::get<bool>(value_);
}
int Node::AsInt() const {
    if (!IsInt()) throw std::logic_error("NO int"s);
    return std::get<int>(value_);
}
double Node::AsDouble() const {
    if (!IsDouble()) throw std::logic_error("No double or int"s);
    if(IsPureDouble()){
        return std::get<double>(value_);
    } else {
        return std::get<int>(value_);
    }
}
const std::string& Node::AsString() const {
    if (!IsString()) throw std::logic_error("No string"s);
    return std::get<std::string>(value_);
}
    
bool Node::operator==(const Node& other) const {
    return value_ == other.value_;
}
bool Node::operator!=(const Node& other) const {
    return value_ != other.value_;
}
    
namespace print {
    
void PrintNode(const Node& node, std::ostream& out);

template <typename Value>
void PrintValue(const Value& value, std::ostream& out) {
    out << value;
}

// Перегрузка для вывода null
void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"s;
}

// Перегрузка для вывода bool
void PrintValue(bool value, std::ostream& out) {
    out << (value ? "true"s : "false"s);
}

// Перегрузка для вывода строк
void PrintValue(const std::string& value, std::ostream& out) {
    out << '"';
    for (const char ch : value) {
        switch (ch) {
            case '\n':
                out << "\\n"s;
                break;
            case '\t':
                out << "\\t"s;
                break;
            case '\r':
                out << "\\r"s;
                break;
            case '"':
                out << "\\\""s;
                break;
            case '\\':
                out << "\\\\"s;
                break;
            default:
                out << ch;
                break;
        }
    }
    out << '"';
}

// Функция для вывода массива 
void PrintValue(const Array& array, std::ostream& out) {
    out << "[\n"s;
    bool first = true;
    std::string indent_str(4, ' ');  
    for (const auto& elem : array) {
        if (!first) {
            out << ",\n"s;
        }
        out << indent_str;
        PrintNode(elem, out);  
        first = false;
    }
    out << "\n"s << ' ' << "]"s;
}

// Функция для вывода словаря 
void PrintValue(const Dict& dict, std::ostream& out) {
    out << "{\n"s;
    bool first = true;
    std::string indent_str(4, ' ');  
    for (const auto& [key, value] : dict) {
        if (!first) {
            out << ",\n"s;
        }
        out << indent_str << '"' << key << "\": "s;
        PrintNode(value, out);  
        first = false;
    }
    out << "\n"s << ' ' << "}"s;
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value) { PrintValue(value, out); },
        node.GetValue());
}
} //print
    
Document::Document()
    : root_(nullptr) {}
    
Document::Document(Node root)
    : root_(std::move(root)) {}
    
const Node& Document::GetRoot() const {
        return root_;
}
    
bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}
bool Document::operator!=(const Document& other) const {
    return root_ != other.root_;
}

Document Load(std::istream& input) {
    return Document{load::LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    (void) &doc;
    (void) &output;
    print::PrintNode(doc.GetRoot(), output);
}

}  // namespace json