#include <petunia/ipc_medium.h>
namespace Petunia 
{

  IPCMedium::IPCMedium(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/) : m_channel(channel)
    , m_connection_role(connection_role)
  {

  }

  IPCMedium::~IPCMedium()
  {

  }

  ConnectionRole IPCMedium::GetConnectionRole()
  {
    return m_connection_role;
  }

  const std::string IPCMedium::GetChannel() const
  {
    return m_channel;
  }

}