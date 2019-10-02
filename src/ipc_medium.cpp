#include <petunia/ipc_medium.h>
namespace Petunia 
{
  ConnectionRole IPCMedium::GetConnectionRole()
  {
    return m_connection_role;
  }
}