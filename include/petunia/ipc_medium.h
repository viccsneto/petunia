#pragma once
#include <queue>
#include "types.h"
#include "message.h"
namespace Petunia
{
  class IPCMedium
  {
  public:
    IPCMedium(std::string &channel, ConnectionRole connection_role = ConnectionRole::Auto);


    virtual ~IPCMedium();

    virtual bool ReceiveMessages(std::queue<Message *> &inbox_queue) = 0;
    virtual bool SendMessages(std::queue<Message *> &outbox_queue) = 0;
    ConnectionRole GetConnectionRole();
  protected:
    std::string m_channel;
    ConnectionRole m_connection_role;
  };
}