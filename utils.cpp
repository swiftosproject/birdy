#include <iostream>
#include <fstream>
#include "utils.h"
#include "package_manager.h"
#include <nlohmann/json.hpp>

std::string formatSize(double size)
{
    const char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int suffixIndex = 0;
    while (size >= 1024 && suffixIndex < 3)
    {
        size /= 1024;
        suffixIndex++;
    }
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%.2f %s", size, suffixes[suffixIndex]);
    return std::string(buffer);
}

int progressBar(void *ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
    // credits: https://stackoverflow.com/a/1639047
    if (TotalToDownload <= 0.0)
    {
        return 0;
    }

    int totaldotz = 40;
    double fractiondownloaded = NowDownloaded / TotalToDownload;
    int dotz = (int)round(fractiondownloaded * totaldotz);

    int ii = 0;
    printf("\r%.0f%%[", fractiondownloaded * 100);
    for (; ii < dotz; ii++)
    {
        printf("=");
    }
    for (; ii < totaldotz; ii++)
    {
        printf(" ");
    }
    printf("]");

    std::string nowDownloadedStr = formatSize(NowDownloaded);
    std::string totalToDownloadStr = formatSize(TotalToDownload);

    printf("%s/%s", nowDownloadedStr.c_str(), totalToDownloadStr.c_str());
    printf("\033[K");
    fflush(stdout);
    return 0;
}

void writeExtractedFilesList(const std::string &listPath, const std::vector<std::string> &extractedFiles, std::string packageName, std::string packageVersion)
{
    nlohmann::json j;
    std::ifstream infile(listPath);
    if (infile.is_open())
    {
        infile >> j;
        infile.close();
    }

    nlohmann::json packageInfo;
    packageInfo["name"] = packageName;
    packageInfo["version"] = packageVersion;
    packageInfo["files"] = extractedFiles;

    j.push_back(packageInfo);

    std::ofstream outfile(listPath);
    if (outfile.is_open())
    {
        outfile << j.dump(4);
        outfile.close();
    }
    else
    {
        std::cerr << "Could not open file for writing: " << listPath << std::endl;
    }
}

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

bool isPackageInstalled(const std::string &listPath, const std::string &packageName)
{
    return isPackageInstalled(listPath, packageName, "_any");
}

std::vector<std::string> getPackageFiles(const std::string &listPath, std::string packageName)
{
    nlohmann::json j;
    std::ifstream infile(listPath);

    if (infile.is_open())
    {
        try
        {
            infile >> j;
        }
        catch (const nlohmann::json::parse_error &e)
        {
            std::cerr << "nlohmann::json parse error: " << e.what() << std::endl;
        }
        infile.close();
    }
    else
    {
        std::cerr << "Could not open file for reading: " << listPath << std::endl;
    }

    for (const auto &packageInfo : j)
    {
        if (packageInfo["name"] == packageName)
        {
            return packageInfo["files"];
        }
    }
}
