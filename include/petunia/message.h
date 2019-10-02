#pragma once
#include <string>
namespace Petunia
{
  class Message
  {
  public:
    Message(std::string msg_type, std::string msg_text);
    Message(std::string msg_type, size_t msg_size, void *msg_data);
    Message(std::string msg_type, std::string msg_text, size_t msg_size, void *msg_data);
    Message(std::string msg_type, std::string msg_text, std::string msg_data);
    Message(const char *msg_type, const char *msg_text, const char *msg_data);
    const char         *GetType() const;
    unsigned long long  GetTextSize() const;
    const char         *GetText() const;
    unsigned long long  GetDataSize() const;
    const void         *GetData() const;
    ~Message();
    void SetOverwriteMode(bool overwrite);
    bool GetOverwriteMode();    
  private:
    void       *m_msg_data;
    unsigned long long      m_data_size;
    std::string m_msg_text;
    std::string m_msg_type;
    bool        m_overwrite;
    friend class Petunia;
    friend class IPCMedium;
  };
} // namespace Petunia