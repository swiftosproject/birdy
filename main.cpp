#include <iostream>
#include "argparse/argparse.hpp"
#include "package_manager.h"
#include "network.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    argparse::ArgumentParser parser("Birdy");

    parser.add_argument("-i", "--install")
        .help("Installs a package")
        .nargs(argparse::nargs_pattern::at_least_one);

    parser.add_argument("-r", "--root")
        .help("Sets the root where Birdy will install packages")
        .nargs(1);

    parser.add_argument("-u", "--uninstall")
        .help("Uninstalls a package")
        .nargs(1);

    parser.add_argument("--fetch")
        .help("Fetch a package's file")
        .nargs(4);

    parser.add_argument("--info")
        .help("Get information about a package")
        .nargs(argparse::nargs_pattern::at_least_one);

    parser.add_argument("--server")
        .help("Change the Birdy server")
        .nargs(1);

    try
    {
        parser.parse_args(argc, argv);
        parseArguments(parser);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::cout << parser;
        return 1;
    }

    return 0;
}
