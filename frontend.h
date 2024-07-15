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
PackageInfo fetchPackageInfo(const std::string& packageName, const std::string& packageVersion);
std::string formatSize(double size);
int progressBar(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);
int fetchPackage(const std::string& packageName, const std::string& packageVersion, const std::string& file, const std::string& outputFile);
std::string fetchLatestVersion(const std::string& packageName);
void extractArchive(const std::string &archivePath, const std::string &outputDir, std::vector<std::string> &extractedFiles);
int copyData(struct archive *ar, struct archive *aw);
void writeExtractedFilesList(const std::string &listPath, const std::vector<std::string> &extractedFiles, std::string packageName, std::string packageVersion);
bool isPackageInstalled(const std::string &listPath, const std::string &packageName, const std::string &packageVersion);
bool isPackageInstalled(const std::string &listPath, const std::string &packageName);
std::vector<std::string> getPackageFiles(const std::string &listPath, std::string packageName);