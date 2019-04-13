#pragma once
#include <string>
namespace Petunia
{
  class Message
  {
  public:
    Message(std::string msg_type, std::string msg_text, size_t msg_size, void *msg_data);
    Message(std::string msg_type, std::string msg_text, std::string msg_data);
    Message(const char *msg_type, const char *msg_text, const char *msg_data);
    std::string GetType();
    size_t      GetSize();
    const char *GetText();
    size_t      GetTextSize();
    void *      GetData();
    ~Message();

  private:
    void *      m_msg_data;
    size_t      m_msg_size;
    std::string m_msg_text;
    std::string m_msg_type;
  };
} // namespace Petunia