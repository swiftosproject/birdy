#include <iostream>
#include <fstream>
#include <filesystem>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "package_manager.h"
#include "network.h"
#include "utils.h"
#include "archive_manager.h"
#include "main.h"

int install(std::string package, std::string version)
{
    std::cout << "Installing " << package << std::endl;
    // Fetch package information
    std::vector<std::string> extractedFiles;
    PackageInfo packageInfo = fetchPackageInfo(package, version);
    std::string archivePath = "/tmp/" + packageInfo.files[0];
    std::string packageListPath = root + "etc/birdy/packages.nlohmann::json";

    // Check if already installed
    if (isPackageInstalled(packageListPath, package, version))
    {
        std::cout << "Already up to date!" << std::endl;
        return 2;
    }

    // Install dependencies
    std::cout << "Resolving dependencies..." << std::endl;
    for (const std::string& dependency : packageInfo.dependencies)
    {
        install(dependency);
    }

    // Download package
    std::cout << "Retrieving packages..." << std::endl;
    fetchPackage(package, version, packageInfo.files[0], archivePath);

    // Extract archive
    std::cout << "Extracting archive...";
    extractArchive(archivePath, root, extractedFiles);
    std::cout << "done" << std::endl;

    // Update package list
    writeExtractedFilesList(packageListPath, extractedFiles, package, version);
    std::cout << "Successfully installed " << package << "!" << std::endl;
    return 0;
}

// Gets the latest version and calls install with that version
int install(std::string package)
{
    std::string version = fetchLatestVersion(package);
    return install(package, version);
}

int uninstall(std::string package)
{
    std::string packageListPath = root + "etc/birdy/packages.nlohmann::json";

    // Check if package is installed
    bool installed = isPackageInstalled(packageListPath, package);
    if (!installed)
    {
        std::cout << "Package is not installed!" << std::endl;
        return 2;
    }
    std::vector<std::string> files = getPackageFiles(packageListPath, package);

    // Remove all files related to the package
    for (std::string file : files)
    {
        try
        {
            if (std::filesystem::remove(file))
                std::cout << "File " << file << " deleted.\n";
            else
                std::cout << "File " << file << " not found.\n";
        }
        catch (const std::filesystem::filesystem_error &err)
        {
            std::cout << "Filesystem error: " << err.what() << '\n';
        }
    }

    // TODO Remove it from installed packages list

    return 0;
}

// Displays package information
int displayPackageInfo(std::string package, std::string version)
{
    PackageInfo packageInfo = fetchPackageInfo(package, version);
    std::cout << "ID: " << packageInfo.id << std::endl;
    std::cout << "Name: " << packageInfo.name << std::endl;
    std::cout << "Description: " << packageInfo.description << std::endl;
    std::cout << "Version: " << packageInfo.version << std::endl;
    std::cout << "Dependencies: ";
    for (std::string dependency : packageInfo.dependencies)
    {
        std::cout << dependency << ", ";
    }
    std::cout << std::endl;

    return 0;
}

// Calls displayPackageInfo with the latest version
int displayPackageInfo(std::string package)
{
    displayPackageInfo(package, "latest");
    return 0;
}

std::string fetchLatestVersion(const std::string &packageName)
{
    auto packageInfo = fetchPackageInfo(packageName, "latest");
    return packageInfo.version;
}