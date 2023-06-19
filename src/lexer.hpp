#pragma once

#include <cctype>
#include <deque>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

namespace kccani
{

class Token
{
public:
    enum class TokenType
    {
        // Terminators
        TOKEN_EOF = 1,
        // Commands
        TOKEN_DEF = 2,
        TOKEN_EXTERN = 3,
        // Primary
        TOKEN_IDENTIFIER = 4,
        TOKEN_NUMBER = 5,
        // Operators
        TOKEN_SPECIAL = -1,
    };

    TokenType type;
    std::optional<std::variant<std::string, double, char>> data;

    Token(TokenType _type);
    Token(TokenType _type, std::string _name);
    Token(TokenType _type, double _value);
    Token(TokenType _type, char _operator);

    operator bool();
    [[nodiscard]] bool operator ==(std::string other) const noexcept;
    [[nodiscard]] bool operator ==(double other) const noexcept;
    [[nodiscard]] bool operator ==(char other) const noexcept;
};

Token get_token(std::istream& fin);
std::deque<Token> tokenize(std::istream& fin);

}
