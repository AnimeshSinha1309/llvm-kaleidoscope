#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "lexer.hpp"


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

            auto tokens = kccani::tokenize(fin);
        }
    }
}
