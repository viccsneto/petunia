#pragma once
#include "ipc_medium.h"
#include "message.h"
#include <queue>
#include <memory>
namespace Petunia {
  class IPCInternalMedium;
  class IPCMediumZMQ : public IPCMedium
  {
  public:
    IPCMediumZMQ(std::string &channel, ConnectionRole connection_role = ConnectionRole::Auto);


    ~IPCMediumZMQ();
    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue) override;
    bool SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue) override;
  
  private:
    std::shared_ptr<IPCInternalMedium> m_internal_medium;
  };
}