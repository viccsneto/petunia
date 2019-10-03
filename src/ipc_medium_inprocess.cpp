#include <petunia/ipc_medium_inprocess.h>
#include <assert.h>

namespace Petunia {
  std::unordered_map<std::string, std::queue<Message *> *> IPCMediumInprocess::s_to_server_messages;
  std::unordered_map<std::string, std::queue<Message *> *> IPCMediumInprocess::s_to_client_messages;
  std::mutex IPCMediumInprocess::s_to_server_lock;
  std::mutex IPCMediumInprocess::s_to_client_lock;

  IPCMediumInprocess::IPCMediumInprocess(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/)
    :IPCMedium(channel, connection_role)
  {
    InitializeRole();

  }

  IPCMediumInprocess::~IPCMediumInprocess()
  {
  }

  bool IPCMediumInprocess::SendMessages(std::queue<Message *> &outbox_queue)
  {
    std::lock_guard<std::mutex> lock(*m_send_lock);
    auto send_queue_it = m_send_messages_map->find(m_channel);

    if (!outbox_queue.empty()) {
      outbox_queue.swap(*send_queue_it->second);
      return true;
    }

    return false;
  }


  void IPCMediumInprocess::EnsureToServerChannelExists()
  {
    std::lock_guard<std::mutex> lock(s_to_server_lock);
    if (s_to_server_messages.find(m_channel) == s_to_server_messages.end()) {
      s_to_server_messages.insert_or_assign(m_channel, new std::queue<Message *>());
    }
  }

  void IPCMediumInprocess::EnsureToClientChannelExists()
  {
    std::lock_guard<std::mutex> lock(s_to_client_lock);
    if (s_to_client_messages.find(m_channel) == s_to_client_messages.end()) {
      s_to_client_messages.insert_or_assign(m_channel, new std::queue<Message *>());
    }
  }

  bool IPCMediumInprocess::ReceiveMessages(std::queue<Message *> &inbox_queue)
  {
    std::lock_guard<std::mutex> lock(*m_receive_lock);
    auto receive_queue_it = m_receive_messages_map->find(m_channel);

    if (!receive_queue_it->second->empty()) {
      inbox_queue.swap(*receive_queue_it->second);
      return true;
    }

    return false;
  }

  void IPCMediumInprocess::InitializeRole()
  {
    // If connection role is auto, tries to detect which role it should have. 
    if (m_connection_role == ConnectionRole::Auto) {
      if (s_to_server_messages.find(m_channel) == s_to_server_messages.end()) {
        m_connection_role = ConnectionRole::Server;
      }
      else {
        m_connection_role = ConnectionRole::Client;
      }
    }
    else {
      /*Nothing to do since it is already initialized by the ancestor constructor call*/
    }

    if (m_connection_role == ConnectionRole::Server) {
      m_receive_messages_map = &s_to_server_messages;
      m_send_messages_map = &s_to_client_messages;

      m_receive_lock = &s_to_server_lock;
      m_send_lock    = &s_to_client_lock;
    }
    else {
      m_receive_messages_map = &s_to_client_messages;
      m_send_messages_map = &s_to_server_messages;

      m_receive_lock = &s_to_client_lock;
      m_send_lock = &s_to_server_lock;
    }

    EnsureToServerChannelExists();
    EnsureToClientChannelExists();
  }
}