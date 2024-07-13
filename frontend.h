#include <string>

int main(int argc, char *argv[]);
int install(std::string package);
int uninstall(std::string package);
int getPackageInfo(std::string package);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
int login(const std::string& username, const std::string& password);
