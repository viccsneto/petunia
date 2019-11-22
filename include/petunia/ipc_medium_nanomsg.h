#pragma once
#include "ipc_medium.h"
#include "message.h"
#include <queue>
#include <memory>
namespace Petunia {
  class IPCMediumNanomsg : public IPCMedium
  {
  public:
    IPCMediumNanomsg(std::string &channel, ConnectionRole connection_role = ConnectionRole::Auto);


    ~IPCMediumNanomsg();
    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue) override;
    bool SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue) override;
  
  private:
    std::shared_ptr<IPCMedium> m_internal_medium;
  };
}