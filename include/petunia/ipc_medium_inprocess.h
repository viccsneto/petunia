#pragma once
#include "ipc_medium.h"
#include "message.h"
#include <queue>
#include <unordered_map>
#include <mutex>
namespace Petunia {
  class IPCMediumInprocess : public IPCMedium
  {
  public:
    IPCMediumInprocess(std::string &channel, ConnectionRole connection_role = ConnectionRole::Auto);


    ~IPCMediumInprocess();
    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue) override;
    bool SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue) override;
  private:
    void InitializeRole();
    std::unordered_map<std::string, std::queue<std::shared_ptr<Message>> *> *m_send_messages_map;
    std::unordered_map<std::string, std::queue<std::shared_ptr<Message>> *> *m_receive_messages_map;
    std::mutex *m_receive_lock;
    std::mutex *m_send_lock;
    static std::unordered_map<std::string, std::queue<std::shared_ptr<Message>> *> s_to_server_messages;
    static std::unordered_map<std::string, std::queue<std::shared_ptr<Message>> *> s_to_client_messages;
    static std::mutex s_to_server_lock;
    static std::mutex s_to_client_lock;
    void EnsureToServerChannelExists();
    void EnsureToClientChannelExists();
  };
}