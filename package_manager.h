#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#include <string>
#include <vector>

struct PackageInfo {
    int id;
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

#endif // PACKAGE_MANAGER_H
