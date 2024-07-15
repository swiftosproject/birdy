#include <iostream>
#include <fstream>
#include <filesystem>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "package_manager.h"
#include "network.h"
#include "utils.h"
#include "archive_manager.h"

extern std::string root;

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

// Checks if package is installed
bool isPackageInstalled(const std::string &listPath, const std::string &packageName, const std::string &packageVersion) {
    nlohmann::json j;
    std::ifstream infile(listPath);
    if (!infile.is_open()) {
        std::cerr << "Could not open file for reading: " << listPath << std::endl;
        return false;
    }

    infile >> j;

    for (const auto &packageInfo : j) {
        if (packageInfo["name"] == packageName && (packageVersion == "_any" || packageInfo["version"] == packageVersion)) {
            return true;
        }
    }
    return false;
}

// Calls isPackageInstalled with no version checking
bool isPackageInstalled(const std::string &listPath, const std::string &packageName)
{
    return isPackageInstalled(listPath, packageName, "_any");
}

PackageInfo fetchPackageInfo(const std::string &packageName, const std::string &packageVersion)
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    PackageInfo packageInfo;

    curl = curl_easy_init();
    if (curl)
    {
        std::string url = (serverAddress + "/packages/" + packageName + "/" + packageVersion + ".nlohmann::json");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);

        try
        {
            auto jsonResponse = nlohmann::json::parse(readBuffer);
            packageInfo.id = jsonResponse["id"];
            packageInfo.name = jsonResponse["name"];
            packageInfo.description = jsonResponse["description"];
            packageInfo.version = jsonResponse["version"];
            packageInfo.dependencies = jsonResponse["dependencies"];
            packageInfo.files = jsonResponse["files"];
        }
        catch (const nlohmann::json::parse_error &e)
        {
            std::cerr << "Json parse error: " << e.what() << std::endl;
        }
    }

    return packageInfo;
}

std::string fetchLatestVersion(const std::string &packageName)
{
    auto packageInfo = fetchPackageInfo(packageName, "latest");
    return packageInfo.version;
}