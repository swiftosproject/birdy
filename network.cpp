#include <iostream>
#include <curl/curl.h>
#include "network.h"
#include "utils.h"
#include <nlohmann/json.hpp>

std::string serverAddress;
std::string authToken;

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

        nlohmann::json payload = {{"username", username}, {"password", password}};
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