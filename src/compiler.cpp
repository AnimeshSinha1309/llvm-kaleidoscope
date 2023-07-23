#include <iostream>
#include <fstream>
#include <type_traits>
#include <variant>

#include <boost/program_options.hpp>

#include "lexer.hpp"
#include "ast.hpp"
#include "parser.hpp"
#include "codegen.hpp"


int main(int argc, char* argv[])
{
    std::cout << "Kaliedoscope compiler by Animesh Sinha: KCCANI v0.1.0" << std::endl;

    boost::program_options::options_description options{"Kaliedoscope Args"};
    options.add_options()
        ("help,h", "Displays all the possible commands and flags.")
        ("file,f", boost::program_options::value<std::vector<std::string>>(), "File or list of files to compile.");
    boost::program_options::variables_map parsed_args;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, options),
        parsed_args
    );
    boost::program_options::notify(parsed_args);

    if (parsed_args.count("help"))
    {
        std::cout << "This is an experimental compiler for the LLVM tutorial language" << std::endl;
    }
    
    if (parsed_args.count("file"))
    {
        auto file_names = parsed_args.at("file").as<std::vector<std::string>>();
        for (auto file_name : file_names)
        {
            std::cout << "Compiling: " << file_name << std::endl;
            std::ifstream fin(file_name, std::fstream::in);

            auto lexer = kccani::Lexer(fin);
            auto parser = kccani::Parser(lexer);
            auto asts = parser.fetch_all();

            kccani::CodeGeneratorLLVM codegen;
            for (auto &ast : asts)
                std::visit(std::ref(codegen), std::move(ast));
            std::cout << "Final LLVM Intermediate Representation output:" << std::endl;
            codegen.print();
        }
    }
    else
    {
        kccani::CodeGeneratorLLVM codegen;
        while (true)
        {
            std::cout << "kccani> ";
            std::string input_string;
            getline(std::cin, input_string);
            std::stringstream input_stream(input_string);

            auto lexer = kccani::Lexer(input_stream);
            auto parser = kccani::Parser(lexer);
            auto ast_stream = parser.fetch_all();
            for (auto &ast : ast_stream)
            {
                auto result = std::visit(std::ref(codegen), std::move(ast));
                if (std::holds_alternative<llvm::Function*>(result))
                    std::cout << std::get<llvm::Function*>(result) << std::endl;
                else if (std::holds_alternative<llvm::Value*>(result))
                    std::cout << std::get<llvm::Value*>(result) << std::endl;
            }
            codegen.print();
        }
    }
}
