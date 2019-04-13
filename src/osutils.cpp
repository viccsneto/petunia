#include <chrono>
#include <petunia/osutils.h>
#include <ratio>
#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <unistd.h>
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

  bool OSUtils::FolderExists(std::string path)
  {
    throw std::logic_error("The method or operation is not implemented.");
  }

  bool OSUtils::FileExists(std::string path)
  {
    throw std::logic_error("The method or operation is not implemented.");
  }

#endif
} // namespace Petunia