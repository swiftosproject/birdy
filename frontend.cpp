#include <iostream>
#include "argparse/argparse.hpp"

bool verbose;
std::string root;

int install(std::string package)
{
    std::cout << "Installing package '" << package << "'";
    if (root != "")
    {
        std::cout << " to '" << root << "'\n";
    }

    return 0;
}

int main(int argc, char *argv[])
{
    argparse::ArgumentParser parser("Birdy");

    parser.add_argument("--install")
        .help("Install the package")
        .nargs(1);

    parser.add_argument("--root")
        .help("Set the root of the installation")
        .nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("Enable verbose output")
        .default_value(false)
        .implicit_value(true);

    try
    {
        parser.parse_args(argc, argv);

        if (parser.get<bool>("--verbose"))
        {
            verbose = true;
        }

        if (parser.present("--root"))
        {
            root = parser.get<std::string>("--root");
        }

        if (parser.present("--install"))
        {
            auto installArgs = parser.get<std::vector<std::string>>("--install");
            std::string package = installArgs[0];
            install(package);
        }

    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cout << parser;
        return 1;
    }

    return 0;
}
