#pragma once
#include "ipc_medium.h"
#include "message.h"
#include <queue> 
#include <memory>

namespace Petunia {
  class IPCInternalMedium;
  class IPCMediumDefault: public IPCMedium
  {
  public:
    IPCMediumDefault(std::string &channel, ConnectionRole connection_role = ConnectionRole::Auto);
    ~IPCMediumDefault();
    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue) override;
    bool SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue) override;
  private:        
    std::unique_ptr<IPCMedium> m_internal_medium;
  };
}