#pragma once

#include <iostream>
#include <exception>

namespace kccani
{

class ParsingException : public std::exception
{
    std::string message;

public:
    explicit ParsingException(std::string _message) : message(_message) {}
    ParsingException(ParsingException const&) noexcept = default;
    ParsingException& operator=(ParsingException const&) noexcept = default;
    
    const char* what() const noexcept override
    {
        return message.c_str();
    }
};

}
