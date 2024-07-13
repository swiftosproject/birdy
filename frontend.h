#include <string>

int main(int argc, char *argv[]);
int install(std::string package, std::string version);
int install(std::string package);
int uninstall(std::string package);
int displayPackageInfo(std::string package);
int displayPackageInfo(std::string package, std::string version);
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
int fetchPackage(const std::string& package_name, const std::string& package_version, const std::string& file, const std::string& output_file);
std::string fetchLatestVersion(const std::string& package_name);
void extract_archive(const std::string &archive_path, const std::string &output_dir, std::vector<std::string> &extracted_files);
int copy_data(struct archive *ar, struct archive *aw);
void write_extracted_files_list(const std::string &list_path, const std::vector<std::string> &extracted_files);