#pragma once
#include <string>
#include <memory>

namespace Petunia
{
  class Message
  {
  public: 
    Message(std::string msg_type, std::shared_ptr<std::string>  msg_text);
    Message(std::string msg_type, std::shared_ptr<std::string>  msg_text, size_t size, std::shared_ptr<std::string> msg_data);
    Message(std::string msg_type, size_t msg_size, std::shared_ptr<std::string> msg_data);
    const char         *GetType();
    std::shared_ptr<std::string> GetText();
    unsigned long long  GetDataSize();
    std::shared_ptr<void> GetData();
    ~Message();
    void SetOverwriteMode(bool overwrite);
    bool GetOverwriteMode();    
  private:
    std::shared_ptr<std::string> m_msg_data;
    std::shared_ptr<std::string> m_msg_text;
    size_t m_data_size;
    std::string m_msg_type;
    bool        m_overwrite;
    friend class Petunia;
    friend class IPCMedium;
  };
} // namespace Petunia