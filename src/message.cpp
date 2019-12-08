#include <stdio.h>
#include <string.h>

#include <petunia/message.h>

namespace Petunia
{
  Message::Message(std::string msg_type, std::shared_ptr<std::string> msg_data)
    : m_msg_type(msg_type)
    , m_msg_data(msg_data)
    , m_overwrite(false)
  {
    msg_type.resize(m_msg_type.size() + 1);
  }
  
  const char *Message::GetType()
  {
    return m_msg_type.c_str();
  }

  unsigned long long Message::GetDataSize()
  {
    return m_msg_data->size();
  }

  void Message::SetOverwriteMode(bool overwrite) 
  {
    m_overwrite = overwrite;
  }

  bool Message::GetOverwriteMode()
  {
    return m_overwrite;
  }

  std::shared_ptr<std::string> Message::GetData()
  {
    return m_msg_data;
  }

  

  Message::~Message()
  {
    
  }
} // namespace Petunia