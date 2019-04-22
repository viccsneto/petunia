#include <stdio.h>
#include <string.h>

#include <petunia/message.h>

namespace Petunia
{
  Message::Message(std::string msg_type, std::string msg_text, size_t msg_size, void *msg_data)
  : m_msg_type(msg_type)
  , m_msg_text(msg_text)
  , m_msg_size(msg_size)
  , m_msg_data(msg_data)
  {
  }

  Message::Message(std::string msg_type, std::string msg_text, std::string msg_data)
  : m_msg_type(msg_type)
  , m_msg_text(msg_text)
  , m_msg_size(msg_data.length() + 1)
  , m_msg_data(strdup(msg_data.c_str()))
  {
  }

  Message::Message(const char *msg_type, const char *msg_text, const char *msg_data)
  : m_msg_type(msg_type)
  , m_msg_size(msg_data == nullptr?0:strlen(msg_data) + 1)
  , m_msg_text(msg_text)
  , m_msg_data(strdup(msg_data))
  {
  }

  const char *Message::GetType() const
  {
    return m_msg_type.c_str();
  }

  size_t Message::GetSize() const
  {
    return m_msg_size;
  }

  size_t Message::GetTextSize() const
  {
    return m_msg_text.size();
  }

  const void *Message::GetData() const
  {
    return m_msg_data;
  }

  const char *Message::GetText() const
  {
    return m_msg_text.c_str();
  }

  Message::~Message()
  {
    free(m_msg_data);
  }
} // namespace Petunia