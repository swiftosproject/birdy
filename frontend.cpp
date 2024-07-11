#include <iostream>
#include "argparse/argparse.hpp"

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

    parser.add_argument("-i", "--install")
        .help("installs a package")
        .nargs(1);

    parser.add_argument("-r", "--root")
        .help("sets the root where birdy will install packages")
        .nargs(1);

    try
    {
        parser.parse_args(argc, argv);

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
