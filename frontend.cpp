#include <iostream>
#include "argparse/argparse.hpp"
#include "frontend.h"
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <archive.h>
#include <archive_entry.h>
#include <math.h>

using json = nlohmann::json;

std::string root = "/";
std::string authToken;
std::string serverAddress = "http://localhost:5000";

int main(int argc, char *argv[])
{
    argparse::ArgumentParser parser("Birdy");

    parser.add_argument("-i", "--install")
        .help("installs a package")
        .nargs(argparse::nargs_pattern::at_least_one);

    parser.add_argument("-r", "--root")
        .help("sets the root where birdy will install packages")
        .nargs(1);

    parser.add_argument("-u", "--uninstall")
        .help("uninstalls a package")
        .nargs(1);

    parser.add_argument("--fetch")
        .help("fetch a packages file")
        .nargs(4);

    parser.add_argument("--info")
        .help("get information about a package")
        .nargs(argparse::nargs_pattern::at_least_one);

    parser.add_argument("--server")
        .help("change the birdy server")
        .nargs(1);

    try
    {
        parser.parse_args(argc, argv);

        if (parser.present("--root"))
        {
            root = parser.get<std::string>("--root");
            if (root[root.size() - 1] != '/')
            {
                root += "/";
            }
        }

        if (parser.present("--server"))
        {
            auto serverArgs = parser.get<std::vector<std::string>>("--server");
            serverAddress = serverArgs[0];
        }

        if (parser.present("--install"))
        {
            auto installArgs = parser.get<std::vector<std::string>>("--install");
            if (installArgs.size() == 1)
            {
                install(installArgs[0]);
            }
            else
            {
                install(installArgs[0], installArgs[1]);
            }
        }

        if (parser.present("--uninstall"))
        {
            auto uninstallArgs = parser.get<std::vector<std::string>>("--uninstall");
            uninstall(uninstallArgs[0]);
        }

        if (parser.present("--fetch"))
        {
            auto fetchArgs = parser.get<std::vector<std::string>>("--fetch");
            fetchPackage(fetchArgs[0], fetchArgs[1], fetchArgs[2], fetchArgs[3]);
        }

        if (parser.present("--info"))
        {
            auto infoArgs = parser.get<std::vector<std::string>>("--info");
            if (infoArgs.size() == 1)
            {
                displayPackageInfo(infoArgs[0]);
            }
            else
            {
                displayPackageInfo(infoArgs[0], infoArgs[1]);
            }
        }
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::cout << parser;
        return 1;
    }

    return 0;
}

int install(std::string package, std::string version)
{
    std::cout << "Installing " << package;
    // Get package information
    std::cout << "Fetching package information...";
    std::vector<std::string> extractedFiles;
    PackageInfo packageInfo = fetchPackageInfo(package, version);
    std::string archivePath = "/tmp/" + packageInfo.files[0];
    std::string file = packageInfo.files[0];
    std::string packageListPath = root + "etc/birdy/packages.json";
    std::cout << "done" << std::endl;

    // Check if this version is already installed
    bool installed = isPackageInstalled(packageListPath, package, version);
    if (installed)
    {
        std::cout << "Already up to date!" << std::endl;
        return 2;
    }

    // Install dependencies
    std::cout << "Resolving dependencies..." << std::endl;
    for (std::string dependency : packageInfo.dependencies)
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
    std::cout << "Updating package list...";
    writeExtractedFilesList(packageListPath, extractedFiles, package, version);
    std::cout << "done" << std::endl;
    return 0;
}

int install(std::string package)
{
    std::string version = fetchLatestVersion(package);
    return install(package, version);
}

int uninstall(std::string package)
{
    std::string packageListPath = root + "etc/birdy/packages.json";

    // Check if package is installed
    bool installed = isPackageInstalled(packageListPath, package);
    if (!installed)
    {
        std::cout << "Package is not installed!" << std::endl;
        return 2;
    }
    std::vector<std::string> files = getPackageFiles(packageListPath, package);

    // Remove all files related to the package and remove it from installed packages list
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

    return 0;
}

int displayPackageInfo(std::string package)
{
    PackageInfo packageInfo = fetchPackageInfo(package, "latest");
    std::cout << "ID: " << packageInfo.id << std::endl;
    std::cout << "Name: " << packageInfo.name << std::endl;
    std::cout << "Description: " << packageInfo.description << std::endl;
    std::cout << "Version: " << packageInfo.version << std::endl;
    std::cout << "Dependencies: ";
    if (!packageInfo.dependencies.empty())
    {
        std::copy(std::begin(packageInfo.dependencies), std::prev(std::end(packageInfo.dependencies)), std::ostream_iterator<std::string>(std::cout, ", "));
        std::cout << packageInfo.dependencies.back();
    }
    std::cout << std::endl;
    std::cout << "Files: ";
    if (!packageInfo.files.empty())
    {
        std::copy(std::begin(packageInfo.files), std::prev(std::end(packageInfo.files)), std::ostream_iterator<std::string>(std::cout, ", "));
        std::cout << packageInfo.files.back();
    }
    std::cout << std::endl;

    return 0;
}

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

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

int login(const std::string &username, const std::string &password)
{
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        std::string readBuffer;
        const char *const url = (serverAddress + "/login").c_str();
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        json payload = {{"username", username}, {"password", password}};
        std::string payload_str = payload.dump();

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload_str.size());

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt");
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookies.txt");

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code == 200)
            {
                authToken = readBuffer;
                std::cout << "\033[32mLogin successful\033[0m" << std::endl;
            }
            else
            {
                std::cerr << "\033[31mLogin failed\033[0m" << std::endl;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
    return 0;
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
        std::string url = (serverAddress + "/packages/" + packageName + "/" + packageVersion + ".json");
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
            auto jsonResponse = json::parse(readBuffer);
            packageInfo.id = jsonResponse["id"];
            packageInfo.name = jsonResponse["name"];
            packageInfo.description = jsonResponse["description"];
            packageInfo.version = jsonResponse["version"];
            packageInfo.dependencies = jsonResponse["dependencies"];
            packageInfo.files = jsonResponse["files"];
        }
        catch (const json::parse_error &e)
        {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }

    return packageInfo;
}

std::string formatSize(double size)
{
    const char *suffixes[] = {"B", "KB", "MB", "GB"};
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

int fetchPackage(const std::string &packageName, const std::string &packageVersion, const std::string &file, const std::string &outputFile)
{
    CURL *curl;
    CURLcode res;
    FILE *fp = fopen(outputFile.c_str(), "wb");
    if (!fp)
    {
        std::cerr << "Failed to open file: " << outputFile << std::endl;
        return 1;
    }

    curl = curl_easy_init();
    if (curl)
    {
        std::string url = (serverAddress + "/packages/" + packageName + "/" + packageVersion + "/" + file);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressBar);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            fclose(fp);
            return 1;
        }

        curl_easy_cleanup(curl);
    }

    fclose(fp);
    std::cout << std::endl;
    return 0;
}

std::string fetchLatestVersion(const std::string &packageName)
{
    auto packageInfo = fetchPackageInfo(packageName, "latest");
    return packageInfo.version;
}

int copyData(struct archive *ar, struct archive *aw)
{
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;)
    {
        int r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK)
        {
            std::cerr << "archive_write_data_block() failed: " << archive_error_string(aw) << std::endl;
            return (r);
        }
    }
}

void extractArchive(const std::string &archivePath, const std::string &outputDir, std::vector<std::string> &extractedFiles)
{
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_compression_all(a);
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
    archive_write_disk_set_standard_lookup(ext);
    if ((r = archive_read_open_filename(a, archivePath.c_str(), 10240)))
    {
        std::cerr << "Failed to open archive: " << archive_error_string(a) << std::endl;
        return;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
        const char *currentFile = archive_entry_pathname(entry);
        std::string fullOutputPath = outputDir;
        if (!fullOutputPath.empty() && fullOutputPath[fullOutputPath.size() - 1] != '/')
        {
            fullOutputPath += "/";
        }
        fullOutputPath += currentFile;
        archive_entry_set_pathname(entry, fullOutputPath.c_str());

        if (archive_entry_filetype(entry) == AE_IFREG)
        {
            extractedFiles.push_back(fullOutputPath);
        }

        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK)
        {
            std::cerr << "archive_write_header() failed: " << archive_error_string(ext) << std::endl;
        }
        else
        {
            copyData(a, ext);
            r = archive_write_finish_entry(ext);
            if (r != ARCHIVE_OK)
            {
                std::cerr << "archive_write_finish_entry() failed: " << archive_error_string(ext) << std::endl;
            }
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
}

void writeExtractedFilesList(const std::string &listPath, const std::vector<std::string> &extractedFiles, std::string packageName, std::string packageVersion)
{
    json j;
    std::ifstream infile(listPath);
    if (infile.is_open())
    {
        infile >> j;
        infile.close();
    }

    json packageInfo;
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
    json j;
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
    json j;
    std::ifstream infile(listPath);

    if (infile.is_open())
    {
        try
        {
            infile >> j;
        }
        catch (const json::parse_error &e)
        {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
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