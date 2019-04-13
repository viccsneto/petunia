#include <petunia/message_handler.h>

namespace Petunia
{
  MessageHandler::MessageHandler(const char *name, std::function<void(const char *text, size_t size, void *data)> listener_function)
  {
    m_name = std::string(name);
    m_listener_function = listener_function;
  }

  std::function<void(const char *text, size_t size, void *data)> MessageHandler::GetFunction()
  {
    return m_listener_function;
  }

  std::string MessageHandler::GetName()
  {
    return m_name;
  }

} // namespace Petunia