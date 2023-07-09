#include "json.h"

using namespace std;

namespace json {

    namespace {
        // ---------- Load ------------------
        using Number = std::variant<int, double>;

        Node LoadNode(istream& input);

        string LoadWord(istream& input) {
            string result;
            while (std::isalpha(static_cast<unsigned char>(input.peek())))
                result.push_back(static_cast<char>(input.get()));

            return result;
        }

        Node LoadArray(istream& input) {
            Array result;
            if (input.peek() == -1) { throw ParsingError("Wrong symbols - expected array"); }

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            if (input.peek() == -1) { throw ParsingError("Wrong symbols - expected Map"); }

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            return Node(move(result));
        }

        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
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
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadBool(istream& input) {
            bool value;
            string c = LoadWord(input);
            if (c == "false") {
                value = false;
                return Node(value);
            }
            else if (c == "true") {
                value = true;
                return Node(value);
            }
            else {
                throw ParsingError("Wrong symbols - expected bool");
            }
        }
   

        Node LoadNull(istream& input) {
            string c = LoadWord(input);
            if (c == "null") {
                return Node(nullptr);
            }
            else {
                throw ParsingError("Wrong symbols - expected null");
            }
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
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
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
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
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("Input is not working");
            }
            //input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    // ---------- Node ------------------

    Node::Node(std::nullptr_t)
        : value_(nullptr) {}

    Node::Node(Array array)
        : value_(std::move(array)) {}

    Node::Node(Dict map)
        : value_(std::move(map)) {}

    Node::Node(bool value)
        : value_(value) {}

    Node::Node(int value)
        : value_(value) {}

    Node::Node(double value)
        : value_(value) {}

    Node::Node(std::string value)
        : value_(std::move(value)) {}

    bool Node::IsInt() const {
        return holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return holds_alternative<double>(value_) || holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value_);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return holds_alternative<string>(value_);
    }

    bool Node::IsNull() const {
        return holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("not an int");
        }
        return std::get<int>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("not a bool");
        }
        return std::get<bool>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("not a double");
        }
        if (IsInt()) { return static_cast<double>(std::get<int>(value_)); }
        return std::get<double>(value_);
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("not a string");
        }
        return std::get<string>(value_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("not an array");
        }
        return std::get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("not a map");
        }
        return std::get<Dict>(value_);
    }

    const Node::Value& Node::GetValue() const {
        return value_;
    }

    bool Node::operator==(const Node& rhs) const {
        return value_ == rhs.value_;
    }
    bool Node::operator!=(const Node& rhs) const {
        return !(value_ == rhs.value_);
    }


    // ---------- Document ------------------
    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& rhs) const {
        return root_ == rhs.root_;
    }
    bool Document::operator!=(const Document& rhs) const {
        return !(root_ == rhs.root_);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    // ---------- Print ------------------
    void OstreamNodePrinter::operator()(std::nullptr_t) {
        out << "null"sv;
    }
    void OstreamNodePrinter::operator()(std::string value) {
        out << "\""sv;
        for (const char& c : value) {
            if (c == '\n') {
                out << "\\n"sv;
                continue;
            }
            if (c == '\r') {
                out << "\\r"sv;
                continue;
            }
            if (c == '\"') out << "\\"sv;
            if (c == '\t') {
                out << "\t"sv;
                continue;
            }
            if (c == '\\') out << "\\"sv;
            out << c;
        }
        out << "\""sv;
    }
    void OstreamNodePrinter::operator()(int value) {
        out << value;
    }
    void OstreamNodePrinter::operator()(double value) {
        out << value;
    }
    void OstreamNodePrinter::operator()(bool value) {
        out << std::boolalpha << value;
    }
    void OstreamNodePrinter::operator()(Array array) {
        out << "[\n"sv;
        bool first = true;
        for (const auto& elem : array) {
            if (first) first = false;
            else out << ", \n"s;

            std::visit(OstreamNodePrinter{ out }, elem.GetValue());
        }
        out << "\n"sv;
        out << "]"sv;
    }
    void OstreamNodePrinter::operator()(Dict dict) {
        out << "{\n"sv;
        bool first = true;
        for (auto& [key, node] : dict) {
            if (first) first = false;
            else out << ", \n"s;
            out << "\"" << key << "\": ";
            std::visit(OstreamNodePrinter{ out }, node.GetValue());
        }
        out << "\n"sv;
        out << " }"sv;
    }

    void Print(const Document& doc, std::ostream& output) {
        std::visit(OstreamNodePrinter{ output }, doc.GetRoot().GetValue());
    }

}  // namespace json