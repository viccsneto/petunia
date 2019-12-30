#pragma once
#include <string>
#include <memory>

namespace Petunia
{
  class Message
  {
  public: 
    Message(std::string msg_type, std::shared_ptr<std::string> msg_data);
    Message(std::string msg_type, std::string msg_data);
    const char         *GetType();
    unsigned long long  GetDataSize();
    std::shared_ptr<std::string> GetData();
    ~Message();
    void SetOverwriteMode(bool overwrite);
    bool GetOverwriteMode();    
  private:
    std::shared_ptr<std::string> m_msg_data;
    std::string m_msg_type;
    bool        m_overwrite;
    friend class Petunia;
    friend class IPCMedium;
  };
} // namespace Petunia