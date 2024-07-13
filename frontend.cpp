#include <iostream>
#include "argparse/argparse.hpp"
#include "frontend.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string root;
std::string auth_token;
const auto server_address = "http://localhost:5000/login";

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

    return 0;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int login(const std::string& username, const std::string& password)
{
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/login");
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

        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code == 200) {
                auth_token = readBuffer;
                std::cout << "\033[32mLogin successful\033[0m" << std::endl;
            } else {
                std::cerr << "\033[31mLogin failed\033[0m" << std::endl;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
    return 0;
}
