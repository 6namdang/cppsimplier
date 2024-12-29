#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <stdexcept>

enum class TokenType {
    OUT,
    IN,
    GREATER_THAN,
    LESS_THAN,
    STRING_LITERAL,
    VAR_NAME,
    STOP,
    END_STATEMENT,
    INT_KEYWORD,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;

    Token(TokenType type, const std::string& value = "") : type(type), value(value) {}
};

class Lexer {
public:
    Lexer(const std::string& input) : input(input), pos(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < input.length()) {
            Token token = next_token();
            if (token.type != TokenType::UNKNOWN) {
                tokens.push_back(token);
            } else {
                throw std::runtime_error("Unknown token encountered");
            }
        }
        return tokens;
    }

private:
    std::string input;
    size_t pos;

    void skip_whitespace() {
        while (pos < input.length() && isspace(input[pos])) {
            pos++;
        }
    }

    Token next_token() {
        skip_whitespace();

        if (pos >= input.length()) {
            return Token(TokenType::END_STATEMENT);
        }

        if (input.substr(pos, 3) == "out") {
            pos += 3;
            return Token(TokenType::OUT, "out");
        }

        if (input.substr(pos, 2) == ">>") {
            pos += 2;
            return Token(TokenType::GREATER_THAN, ">>");
        }

        if (input.substr(pos, 2) == "in") {
            pos += 2;
            return Token(TokenType::IN, "in");
        }

        if (input.substr(pos, 2) == "<<") {
            pos += 2;
            return Token(TokenType::LESS_THAN, "<<");
        }

        if (input.substr(pos, 4) == "stop") {
            pos += 4;
            return Token(TokenType::STOP, "stop");
        }

        if (input.substr(pos, 3) == "int") {
            pos += 3;
            return Token(TokenType::INT_KEYWORD, "int");
        }

        if (input[pos] == '"') {
            pos++;
            size_t start = pos;
            while (pos < input.length() && input[pos] != '"') {
                pos++;
            }
            std::string value = input.substr(start, pos - start);
            pos++;
            return Token(TokenType::STRING_LITERAL, value);
        }

        if (isalpha(input[pos])) {
            size_t start = pos;
            while (pos < input.length() && isalnum(input[pos])) {
                pos++;
            }
            return Token(TokenType::VAR_NAME, input.substr(start, pos - start));
        }

        if (input[pos] == ';') {
            pos++;
            return Token(TokenType::END_STATEMENT, ";");
        }

        pos++;
        return Token(TokenType::UNKNOWN);
    }
};

class Statement {
public:
    virtual void execute(std::unordered_map<std::string, std::string>& variables) = 0;
    virtual ~Statement() = default;
};

class OutStatement : public Statement {
public:
    OutStatement(const std::vector<std::string>& output_targets) : output_targets(output_targets) {}

    void execute(std::unordered_map<std::string, std::string>& variables) override {
        for (const auto& target : output_targets) {
            // If the target is a variable name, look it up in variables
            if (variables.find(target) != variables.end()) {
                std::cout << variables[target] << std::endl;
            } 
            // Otherwise, treat it as a literal string
            else {
                std::cout << target << std::endl;
            }
        }
    }

private:
    std::vector<std::string> output_targets;
};

class InStatement : public Statement {
public:
    InStatement(const std::string& var_name) : var_name(var_name) {}

    void execute(std::unordered_map<std::string, std::string>& variables) override {
        int value;
        std::string input_str;
        std::getline(std::cin, input_str);
        
        try {
            value = std::stoi(input_str);
            variables[var_name] = std::to_string(value);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid input. Please enter an integer." << std::endl;
            variables[var_name] = "0";
        }
    }

private:
    std::string var_name;
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), current_index(0) {}

    std::vector<std::shared_ptr<Statement>> parse() {
        std::vector<std::shared_ptr<Statement>> statements;

        while (current_index < tokens.size()) {
            if (tokens[current_index].type == TokenType::OUT) {
                statements.push_back(parse_out_statement());
            } else if (tokens[current_index].type == TokenType::IN) {
                statements.push_back(parse_in_statement());
            } else {
                current_index++;
            }
        }

        return statements;
    }

private:
    std::vector<Token> tokens;
    size_t current_index;

    std::shared_ptr<Statement> parse_out_statement() {
        current_index++; // Skip 'out'
        std::vector<std::string> output_targets;

        while (current_index < tokens.size() &&
               (tokens[current_index].type == TokenType::GREATER_THAN || 
                tokens[current_index].type == TokenType::STRING_LITERAL || 
                tokens[current_index].type == TokenType::VAR_NAME)) {
            // Skip '>>' if present
            if (tokens[current_index].type == TokenType::GREATER_THAN) {
                current_index++;
                continue;
            }

            // Add string literal or variable name
            if (tokens[current_index].type == TokenType::STRING_LITERAL || 
                tokens[current_index].type == TokenType::VAR_NAME) {
                output_targets.push_back(tokens[current_index].value);
                current_index++;
            }
        }

        if (current_index < tokens.size() &&
            tokens[current_index].type == TokenType::STOP) {
            current_index++; // Skip 'stop'
        }

        return std::make_shared<OutStatement>(output_targets);
    }

    std::shared_ptr<Statement> parse_in_statement() {
        current_index++; // Skip 'in'

        if (current_index < tokens.size() &&
            tokens[current_index].type == TokenType::LESS_THAN) {
            current_index++; // Skip '<<'

            if (current_index < tokens.size() &&
                tokens[current_index].type == TokenType::VAR_NAME) {
                std::string var_name = tokens[current_index].value;
                current_index++;
                return std::make_shared<InStatement>(var_name);
            }
        }

        throw std::runtime_error("Invalid 'in' statement");
    }
};

class Interpreter {
public:
    void execute(const std::vector<std::shared_ptr<Statement>>& statements) {
        std::unordered_map<std::string, std::string> variables;
        for (const auto& stmt : statements) {
            stmt->execute(variables);
        }
    }
};

int main() {
    // Example: Prompt for integer input and then print it out
    std::string source_code = 
        "out >> \"enter a number:\"; in << a; out >> a;";


    Lexer lexer(source_code);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::vector<std::shared_ptr<Statement>> program = parser.parse();

    Interpreter interpreter;
    interpreter.execute(program);

    return 0;
}

