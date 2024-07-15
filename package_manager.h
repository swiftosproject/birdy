#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#include <string>
#include <vector>

extern std::string root;

struct PackageInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string version;
    std::vector<std::string> dependencies;
    std::vector<std::string> files;
};

int install(std::string package);
int install(std::string package, std::string version);
int uninstall(std::string package);
int displayPackageInfo(std::string package);
int displayPackageInfo(std::string package, std::string version);
bool isPackageInstalled(const std::string &listPath, const std::string &packageName, const std::string &packageVersion);
bool isPackageInstalled(const std::string &listPath, const std::string &packageName);
int removePackage(const std::string &packageName);
std::string fetchLatestVersion(const std::string &packageName);
PackageInfo fetchPackageInfo(const std::string &packageName, const std::string &packageVersion);

#endif // PACKAGE_MANAGER_H
