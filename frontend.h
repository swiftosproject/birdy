#include <string>

int main(int argc, char *argv[]);
int install(std::string package);
int uninstall(std::string package);
int displayPackageInfo(std::string package);
struct PackageInfo {
    int id;
    std::string name;
    std::string description;
    std::string version;
    std::vector<std::string> dependencies;
    std::vector<std::string> files;
};
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
int login(const std::string& username, const std::string& password);
PackageInfo fetchPackageInfo(const std::string& package_name, const std::string& package_version);
