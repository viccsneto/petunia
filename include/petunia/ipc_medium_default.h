#pragma once
#include "CppSQLite3.h"
#include "ipc_medium.h"
#include "message.h"
#include <queue>

namespace Petunia {
  class IPCMediumDefault : public IPCMedium
  {
  public:
    IPCMediumDefault(std::string &channel, ConnectionRole connection_role = ConnectionRole::Auto);
    ~IPCMediumDefault();
    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue) override;
    bool SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue) override;
  private:
    void InitializeDatabase();
    void CreateStatements();
    void BeginTransaction();
    void Commit();
    std::shared_ptr<Message> CreateMessageFromRow(CppSQLite3Query &received_messages);
    CppSQLite3Query QueryReceivedMessages();
    void DeleteOldMessages(unsigned long long reference_id);
    void WriteSendingMessage(std::shared_ptr<Message> message);
  private:
    CppSQLite3DB m_ipc_database;
    CppSQLite3Statement m_begin_transaction_stmt;
    CppSQLite3Statement m_commit_transaction_stmt;

    CppSQLite3Statement m_insert_message_stmt;
    CppSQLite3Statement m_update_message_stmt;
    CppSQLite3Statement m_delete_old_messages_stmt;
    CppSQLite3Statement m_select_messages_stmt;
    std::string GenerateChannelPath();
    std::string m_channel_path;
    void TryDeleteChannel();
  };
}