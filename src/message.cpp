#include <stdio.h>
#include <string.h>

#include <petunia/message.h>

namespace Petunia
{
  Message::Message(std::string msg_type, std::shared_ptr<std::string> msg_text)
  : m_msg_type(msg_type)
  , m_msg_text(msg_text)
  , m_data_size(0)
  , m_overwrite(false)
  {
  }

  Message::Message(std::string msg_type,  size_t msg_size, std::shared_ptr<std::string> msg_data)
    : m_msg_type(msg_type)
    , m_data_size(msg_size)
    , m_msg_data(msg_data)
    , m_overwrite(false)
  {
  }

  Message::Message(std::string msg_type, std::shared_ptr<std::string> msg_text, size_t msg_size, std::shared_ptr<std::string> msg_data)
    : m_msg_type(msg_type)
    , m_msg_text(msg_text)
    , m_data_size(msg_size)
    , m_msg_data(msg_data)
    , m_overwrite(false)
  {

  }

  const char *Message::GetType()
  {
    return m_msg_type.c_str();
  }

  unsigned long long Message::GetDataSize()
  {
    return m_data_size;
  }

  void Message::SetOverwriteMode(bool overwrite) 
  {
    m_overwrite = overwrite;
  }

  bool Message::GetOverwriteMode()
  {
    return m_overwrite;
  }

  std::shared_ptr<void> Message::GetData()
  {
    return m_msg_data;
  }

  std::shared_ptr<std::string> Message::GetText() 
  {
    return m_msg_text;
  }

  Message::~Message()
  {
    
  }
} // namespace Petunia