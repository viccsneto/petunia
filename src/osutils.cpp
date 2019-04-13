#include <chrono>

#include <petunia/osutils.h>

#ifdef _WIN32
#  include <process.h>
#  include <windows.h>
#else
#  include <sys/stat.h>
#  include <unistd.h>
#endif

namespace Petunia
{
#ifdef _WIN32
  std::string OSUtils::GetTemporaryFolder()
  {
    std::string temp = std::string(getenv("tmp"));
    
    if (temp[temp.length() - 1] != '\\') {
      temp += "\\";
    }

    return temp;
  }

  bool OSUtils::FileExists(std::string &path)
  {
    struct stat looking_stat;
    if (stat(path.c_str(), &looking_stat) == 0) {
      return looking_stat.st_mode & _S_IFREG == _S_IFREG;
    }

    return false;
  }

  bool OSUtils::FolderExists(std::string &path)
  {
    struct stat looking_stat;
    if (stat(path.c_str(), &looking_stat) == 0) {
      return looking_stat.st_mode & _S_IFDIR == _S_IFDIR;
    }

    return false;
  }

  bool OSUtils::CreateFolder(std::string &path)
  {
    return CreateDirectory(path.c_str(), nullptr) || ERROR_ALREADY_EXISTS == GetLastError();
  }

#else
  std::string OSUtils::GetTemporaryFolder()
  {
    return "/var/tmp/";
  }

  bool OSUtils::FileExists(std::string &path)
  {
    struct stat looking_stat;
    if (stat(path.c_str(), &looking_stat) == 0) {
      return looking_stat.st_mode & S_IFREG == S_IFREG;
    }

    return false;
  }

  bool OSUtils::FolderExists(std::string &path)
  {
    struct stat looking_stat;
    if (stat(path.c_str(), &looking_stat) == 0) {
      return looking_stat.st_mode & S_IFDIR == S_IFDIR;
    }

    return false;
  }

  bool OSUtils::CreateFolder(std::string &path)
  {
    if (FolderExists(path)) {
      return true;
    }

    return mkdir(path.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != -1;
  }

#endif
} // namespace Petunia