#include <iostream>
#include "argparse/argparse.hpp"
// #include <curlpp/cURLpp.hpp>
// #include <curlpp/Options.hpp>

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

int uninstall(std::string package)
{
    std::cout << "Uninstalling package " << package;
    if(root != "")
    {
        std::cout << " from" << root << "\n";
    }

    return 0;
}

int getPackageInfo(std::string package)
{
    std::cout << "ID: ";
    std::cout << "Name: ";
    std::cout << "Description: ";
    std::cout << "Versions: ";
    std::cout << "Dependencies: ";
    std::cout << "Files: ";
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

    parser.add_argument("-u", "--uninstall")
        .help("uninstalls a package")
        .nargs(1);

    parser.add_argument("--info")
    .help("get information about a package")
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
            install(installArgs[0]);
        }

        if (parser.present("--uninstall"))
        {
            auto uninstallArgs = parser.get<std::vector<std::string>>("--uninstall");
            uninstall(uninstallArgs[0]);
        }

    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cout << parser;
        return 1;
    }

    return 0;
}
