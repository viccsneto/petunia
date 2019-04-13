#pragma once
#include <functional>
#include <string>
namespace Petunia
{
  class MessageHandler
  {
  public:
    MessageHandler(const char *name, std::function<void(const char *text, size_t size, void *data)> listener);
    std::function<void(const char *text, size_t size, void *data)> GetFunction();
    std::string                                                    GetName();

  private:
    std::string                                                    m_name;
    std::function<void(const char *text, size_t size, void *data)> m_listener_function;
  };
} // namespace Petunia