#pragma once
#include <string>
namespace Petunia
{
  class OSUtils
  {
  public:    
    static bool FileExists(std::string &looking_path);
    static bool FolderExists(std::string &looking_path);
    static bool CreateFolder(std::string &path);
    static std::string        GetTemporaryFolder();

  public:
  };

} // namespace Petunia