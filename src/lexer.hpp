#include <iostream>
#include <optional>

class Token
{
    enum class TokenType
    {
        // Terminators
        TOKEN_EOF = 1,
        // Commands
        TOKEN_DEF = 2,
        TOKEN_EXTERN = 3,
        // Primary
        TOKEN_IDENTIFIER = 4,
        TOKER_NUMBER = 5,
    };

    TokenType type;
    std::optional<std::string> name;

    Token(TokenType _type, std::optional<std::string> _name) : name(_name), type(_type) {}

public:
    static Token get_token(std::ifstream&& fin)
    {
        return {TokenType::TOKEN_EOF, ""};
    }
};
