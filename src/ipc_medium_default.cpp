#include <petunia/ipc_medium_default.h>
#include <petunia/petunia.h>
#include <petunia/osutils.h>
#include <assert.h>
#include <sqlite3/CppSQLite3.h>

#define CHANNEL_FILE_EXTENSION ".ipc.db"
#define MAX_CHANNEL_DELETE_TRIES 1

namespace Petunia {  
  class IPCInternalMedium : public IPCMedium
  {
  public:
    IPCInternalMedium(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/)
      :IPCMedium(channel, connection_role)
    {
      m_channel_path = GenerateChannelPath();
      InitializeDatabase();
    }

    void InitializeDatabase()
    {
      m_ipc_database.open(m_channel_path.c_str());

      m_ipc_database.execDML("PRAGMA synchronous = OFF");
      m_ipc_database.execDML("PRAGMA journal_mode = OFF");
      m_ipc_database.execDML("PRAGMA mmap_size=44194304");
      m_ipc_database.execDML("PRAGMA busy_timeout=30000");

      bool database_already_created = m_ipc_database.tableExists("to_client");

      if (m_connection_role == ConnectionRole::Auto)
      {
        if (!database_already_created)
        {
          m_connection_role = ConnectionRole::Server;
        }
        else
        {
          m_connection_role = ConnectionRole::Client;
        }
      }

      if (!database_already_created)
      {
        m_ipc_database.execDML("CREATE TABLE to_client(type TEXT, size NUMBER, blob_message BLOB, row_id INTEGER PRIMARY KEY)");
        m_ipc_database.execDML("CREATE TABLE to_server(type TEXT, size NUMBER, blob_message BLOB, row_id INTEGER PRIMARY KEY)");
        m_ipc_database.execDML("CREATE INDEX idx_to_client_type ON to_client(type)");
        m_ipc_database.execDML("CREATE INDEX idx_to_server_type ON to_server(type)");
      }

      CreateStatements();
    }

    ~IPCInternalMedium()
    {
      m_begin_transaction_stmt.finalize();
      m_commit_transaction_stmt.finalize();
      m_insert_message_stmt.finalize();
      m_update_message_stmt.finalize();
      m_delete_old_messages_stmt.finalize();
      m_select_messages_stmt.finalize();
      m_ipc_database.close();

      if (m_connection_role == ConnectionRole::Server) {
        TryDeleteChannel();
      }
    }

    void CreateStatements()
    {
      m_begin_transaction_stmt = m_ipc_database.compileStatement("BEGIN TRANSACTION");
      m_commit_transaction_stmt = m_ipc_database.compileStatement("COMMIT");

      if (m_connection_role == ConnectionRole::Server) {
        m_insert_message_stmt = m_ipc_database.compileStatement("INSERT INTO to_client "
          "("
          "   type,"
          "   size,"
          "   blob_message"
          ")"
          "VALUES"
          "("
          "   @type,"
          "   @size,"
          "   @blob_message"
          ")");

        m_update_message_stmt = m_ipc_database.compileStatement("UPDATE to_client SET "
          "   size = @size,"
          "   blob_message = @blob_message "
          "WHERE "
          "   type = @type");

        m_delete_old_messages_stmt = m_ipc_database.compileStatement("DELETE FROM to_server WHERE row_id <= @row_id");
        m_select_messages_stmt = m_ipc_database.compileStatement("SELECT * FROM to_server");
      }
      else {
        m_insert_message_stmt = m_ipc_database.compileStatement("INSERT INTO to_server "
          "("
          "   type,"
          "   size,"
          "   blob_message"
          ")"
          "VALUES"
          "("
          "   @type,"
          "   @size,"
          "   @blob_message"
          ")");

        m_update_message_stmt = m_ipc_database.compileStatement("UPDATE to_server SET "
          "   size = @size,"
          "   blob_message = @blob_message "
          "WHERE "
          "   type = @type");

        m_delete_old_messages_stmt = m_ipc_database.compileStatement("DELETE FROM to_client WHERE row_id <= @row_id");
        m_select_messages_stmt = m_ipc_database.compileStatement("SELECT * FROM to_client");
      }
    }

    void BeginTransaction()
    {
      m_begin_transaction_stmt.reset();
      m_begin_transaction_stmt.execDML();
    }

    void Commit()
    {
      m_commit_transaction_stmt.reset();
      m_commit_transaction_stmt.execDML();
    }

    std::shared_ptr<Message> CreateMessageFromRow(CppSQLite3Query &received_messages)
    {
      int size = received_messages.getSizeTField("size");
      std::shared_ptr<std::string> buffer = std::make_shared<std::string>();
      const unsigned char *message = received_messages.getBlobField("blob_message", size);
      buffer->resize(size);
      memcpy((void *)buffer->data(), message, size);

      return std::make_shared<Message>(std::string(received_messages.getStringField("type")), buffer);
    }

    CppSQLite3Query QueryReceivedMessages()
    {
      m_select_messages_stmt.reset();
      CppSQLite3Query query_result = m_select_messages_stmt.execQuery();

      return query_result;
    }

    void DeleteOldMessages(unsigned long long reference_id)
    {
      BeginTransaction();
      m_delete_old_messages_stmt.reset();
      m_delete_old_messages_stmt.bindInt64("@row_id", reference_id);
      m_delete_old_messages_stmt.execDML();
      Commit();
    }

    bool SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue)
    {
      if (!outbox_queue.empty()) {

        BeginTransaction();
        while (!outbox_queue.empty()) {
          std::shared_ptr<Message> message = outbox_queue.front();

          WriteSendingMessage(message);

          outbox_queue.pop();
        }
        Commit();
        return true;
      }

      return false;
    }

    void WriteSendingMessage(std::shared_ptr<Message> message)
    {
      if (message->GetOverwriteMode()) {
        m_update_message_stmt.reset();
        m_update_message_stmt.bind("@type", message->GetType());
        m_update_message_stmt.bindInt64("@size", message->GetDataSize());
        m_update_message_stmt.bind("@blob_message", (const unsigned char *)message->GetData()->data(), message->GetDataSize());
        if (m_update_message_stmt.execDML() != 0) {
          return;
        }
      }

      m_insert_message_stmt.reset();
      m_insert_message_stmt.bind("@type", message->GetType());
      m_insert_message_stmt.bindInt64("@size", message->GetDataSize());
      m_insert_message_stmt.bind("@blob_message", (const unsigned char *)message->GetData()->data(), message->GetDataSize());
      m_insert_message_stmt.execDML();
    }

    std::string GenerateChannelPath()
    {
      std::string petunia_folder_path = Petunia::GetPetuniaFolder();

      return petunia_folder_path + m_channel + CHANNEL_FILE_EXTENSION;
    }

    void TryDeleteChannel()
    {
      for (size_t i = 0; i < MAX_CHANNEL_DELETE_TRIES; ++i) {
        if (remove(m_channel_path.c_str()) == 0) {
          break;
        }
      }
    }

    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue)
    {
      unsigned long long delete_reference_id = 0;

      CppSQLite3Query received_messages = QueryReceivedMessages();

      while (!received_messages.eof()) {
        delete_reference_id = received_messages.getInt64Field("row_id");

        std::shared_ptr<Message> message = CreateMessageFromRow(received_messages);

        inbox_queue.push(message);
        received_messages.nextRow();
      }

      if (delete_reference_id > 0) {
        DeleteOldMessages(delete_reference_id);

        return true;
      }

      return false;
    }

  private:
    CppSQLite3DB m_ipc_database;
    CppSQLite3Statement m_begin_transaction_stmt;
    CppSQLite3Statement m_commit_transaction_stmt;

    CppSQLite3Statement m_insert_message_stmt;
    CppSQLite3Statement m_update_message_stmt;
    CppSQLite3Statement m_delete_old_messages_stmt;
    CppSQLite3Statement m_select_messages_stmt;
    std::string m_channel_path;
  };

  IPCMediumDefault::IPCMediumDefault(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/)
    :IPCMedium(channel, connection_role)
  {
    m_internal_medium =  std::make_shared<IPCInternalMedium>(channel, connection_role);
  }

  IPCMediumDefault::~IPCMediumDefault()
  {
  }

  bool IPCMediumDefault::ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue)
  {
    return m_internal_medium->ReceiveMessages(inbox_queue);
  }

  bool IPCMediumDefault::SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue)
  {
    return m_internal_medium->SendMessages(outbox_queue);
  }

}