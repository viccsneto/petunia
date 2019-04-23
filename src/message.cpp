#include <stdio.h>
#include <string.h>

#include <petunia/message.h>

namespace Petunia
{
  Message::Message(std::string msg_type, std::string msg_text, size_t msg_size, void *msg_data)
  : m_msg_type(msg_type)
  , m_msg_text(msg_text)
  , m_data_size(msg_size)
  , m_msg_data(msg_data)
  , m_overwrite(false)
  {
  }

  Message::Message(std::string msg_type, std::string msg_text, std::string msg_data)
  : m_msg_type(msg_type)
  , m_msg_text(msg_text)
  , m_data_size(msg_data.length() + 1)
  , m_msg_data(strdup(msg_data.c_str()))
  , m_overwrite(false)
  {
  }

  Message::Message(const char *msg_type, const char *msg_text, const char *msg_data)
  : m_msg_type(msg_type)
  , m_data_size(msg_data == nullptr?0:strlen(msg_data) + 1)
  , m_msg_text(msg_text)
  , m_msg_data(msg_data == nullptr?nullptr:strdup(msg_data))
  , m_overwrite(false)
  {
  }

  const char *Message::GetType() const
  {
    return m_msg_type.c_str();
  }

  unsigned long long Message::GetDataSize() const
  {
    return m_data_size;
  }

  unsigned long long Message::GetTextSize() const
  {
    return m_msg_text.size();
  }

  void Message::SetOverwriteMode(bool overwrite) 
  {
    m_overwrite = overwrite;
  }

  bool Message::GetOverwriteMode()
  {
    return m_overwrite;
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