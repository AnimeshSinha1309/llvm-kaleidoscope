#include "lexer.hpp"

#include <cassert>
#include <fstream>

namespace kccani
{

Token::Token(TokenType _type) : type(_type), data(std::nullopt)
{
    assert(this->type != TokenType::TOKEN_IDENTIFIER);
    assert(this->type != TokenType::TOKEN_NUMBER);
    assert(this->type != TokenType::TOKEN_SPECIAL);
}

Token::Token(TokenType _type, std::string _name) : type(_type), data(_name)
{
    assert(this->type == TokenType::TOKEN_IDENTIFIER);
}

Token::Token(TokenType _type, double _value) : type(_type), data(_value)
{
    assert(this->type == TokenType::TOKEN_NUMBER);
}

Token::Token(TokenType _type, char _operator) : type(_type), data(_operator)
{
    assert(this->type == TokenType::TOKEN_SPECIAL);
}

Token::operator bool()
{
    return this->type != TokenType::TOKEN_EOF;
}

[[nodiscard]] bool Token::operator ==(std::string other) const noexcept
{
    if (this->type != TokenType::TOKEN_IDENTIFIER)
        return false;
    return std::get<std::string>(this->data.value()) == other;
}

[[nodiscard]] bool Token::operator ==(double other) const noexcept
{
    if (this->type != TokenType::TOKEN_NUMBER)
        return false;
    return std::get<double>(this->data.value()) == other;
}

[[nodiscard]] bool Token::operator ==(char other) const noexcept
{
    if (this->type != TokenType::TOKEN_SPECIAL)
        return false;
    return std::get<char>(this->data.value()) == other;
}


Token Lexer::get()
{
    if (this->buffered_token)
    {
        auto output_value = this->buffered_token.value();
        this->buffered_token.reset();
        return output_value;
    }

    this->input_stream >> std::noskipws;

    while (stream_char == ' ')
        input_stream >> stream_char;

    if (isalpha(stream_char))
    {
        std::string token = "";
        do {
            token += stream_char;
            input_stream >> stream_char;
        } while (isalnum(stream_char));

        if (token == "def")
            return Token::TokenType::TOKEN_DEF;
        else if (token == "extern")
            return Token::TokenType::TOKEN_EXTERN;
        else
            return {Token::TokenType::TOKEN_IDENTIFIER, token};
    }

    if (isdigit(stream_char) || stream_char == '.') {
        std::string token = "";
        do {
            token += stream_char;
            input_stream >> stream_char;
        } while (isdigit(stream_char) || stream_char == '.');

        std::size_t __string_size_v;
        double value = std::stod(token, &__string_size_v);
        return {Token::TokenType::TOKEN_NUMBER, value};
    }

    if (stream_char == '#') {
        do {
            input_stream >> stream_char;
        } while (stream_char != EOF && stream_char != '\n' && stream_char != '\r');

        if (stream_char != EOF)
            return get();
    }

    char current_char = stream_char;
    if (input_stream >> stream_char)
    {
        if (current_char == '\n' || current_char == ' ')
            return this->get();
        return {Token::TokenType::TOKEN_SPECIAL, current_char};
    }
    else
        return Token::TokenType::TOKEN_EOF;
}

Token Lexer::peek()
{
    if (!this->buffered_token)
        this->buffered_token = this->get();
    return this->buffered_token.value();
}

Lexer::Lexer(std::basic_istream<char>& text_stream) : input_stream(text_stream)
{
}

Lexer Lexer::operator>>(Token& output_token)
{
    output_token = this->get();
    return *this;
}

std::queue<Token> Lexer::fetch_all()
{
    std::queue<Token> all_tokens;
    do
    {
        Token token = this->get();
        all_tokens.push(token);
    } while (
        all_tokens.empty() ||
        all_tokens.back().type != Token::TokenType::TOKEN_EOF);
    return all_tokens;
}

}
