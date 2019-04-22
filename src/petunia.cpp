#include <assert.h>
#include <chrono>
#include <iostream>
#include <list>

#include <petunia/petunia.h>
#include <petunia/message.h>
#include <petunia/osutils.h>

#include "CppSQLite3/CppSQLite3.h"

#define CHANNEL_PATH_SUBFOLDER "petunia/"
#define CHANNEL_FILE_EXTENSION ".ipc.db"

namespace Petunia
{
  class IPCMedium
  {
  public:
    IPCMedium(std::string &channel, ConnectionRole connection_role)
      : m_channel(channel)
      , m_connection_role(connection_role)
    {
      InitializeDatabase();
    }

    void InitializeDatabase()
    {
      m_ipc_database.open(m_channel.c_str());

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
        m_ipc_database.execDML("CREATE TABLE to_client(type TEXT, size NUMBER, blob_message BLOB, text_message TEXT, row_id INTEGER PRIMARY KEY)");
        m_ipc_database.execDML("CREATE TABLE to_server(type TEXT, size NUMBER, blob_message BLOB, text_message TEXT, row_id INTEGER PRIMARY KEY)");
        m_ipc_database.execDML("CREATE INDEX idx_to_client_type ON to_client(type)");
        m_ipc_database.execDML("CREATE INDEX idx_to_server_type ON to_server(type)");
      }

      CreateStatements();
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
          "   blob_message,"
          "   text_message"
          ")"
          "VALUES"
          "("
          "   @type,"
          "   @size,"
          "   @blob_message,"
          "   @text_message"
          ")");

        m_update_message_stmt = m_ipc_database.compileStatement("UPDATE to_client SET "
          "   size = @size,"
          "   blob_message = @blob_message,"
          "   text_message = @text_message "
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
          "   blob_message,"
          "   text_message"
          ")"
          "VALUES"
          "("
          "   @type,"
          "   @size,"
          "   @blob_message,"
          "   @text_message"
          ")");

        m_update_message_stmt = m_ipc_database.compileStatement("UPDATE to_server SET "
          "   size = @size,"
          "   blob_message = @blob_message,"
          "   text_message = @text_message "
          "WHERE "
          "   type = @type");

        m_delete_old_messages_stmt = m_ipc_database.compileStatement("DELETE FROM to_client WHERE row_id <= @row_id");
        m_select_messages_stmt = m_ipc_database.compileStatement("SELECT * FROM to_client");
      }
    }

    ~IPCMedium()
    {
      m_ipc_database.close();
      remove(m_channel.c_str());
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

    bool EnqueueReceivedMessages(std::queue<Message *> &inbox_queue)
    {
      unsigned long long delete_reference_id = 0;

      CppSQLite3Query received_messages = QueryReceivedMessages();

      while (!received_messages.eof()) {
        delete_reference_id = received_messages.getInt64Field("row_id");

        Message *message = CreateMessageFromRow(received_messages);

        inbox_queue.push(message);
        received_messages.nextRow();
      }

      if (delete_reference_id > 0) {
        DeleteOldMessages(delete_reference_id);

        return true;
      }

      return false;
    }

    Message *CreateMessageFromRow(CppSQLite3Query &received_messages)
    {
      size_t size = received_messages.getSizeTField("size");
      void *buffer = malloc(size);
      if (!buffer) {
        printf("PANIC!\n");
        assert(false);
        exit(-1);
      }

      int nSize;
      const unsigned char *message = received_messages.getBlobField("blob_message", nSize);
      assert(size == nSize);
      memcpy(buffer, message, size);
      
      return new Message(std::string(received_messages.getStringField("type")), std::string(received_messages.getStringField("text_message")), size, buffer);
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

    bool SendEnqueuedMessages(std::queue<Message *> &outbox_queue)
    {
      if (!outbox_queue.empty())
      {

        BeginTransaction();
        while (!outbox_queue.empty())
        {
          Message *message = outbox_queue.front();

          WriteSendingMessage(message);

          outbox_queue.pop();
          delete message;
        }
        Commit();
        return true;
      }

      return false;
    }

    void WriteSendingMessage(Message * message)
    {
      m_insert_message_stmt.reset();
      m_insert_message_stmt.bind("@type", message->GetType().c_str());
      m_insert_message_stmt.bindSizeT("@size", message->GetSize());
      m_insert_message_stmt.bind("@text_message", (const unsigned char *)message->GetText(), message->GetTextSize());
      m_insert_message_stmt.bind("@blob_message", (const unsigned char *)message->GetData(), message->GetSize());
      m_insert_message_stmt.execQuery();
    }

    ConnectionRole GetConnectionRole()
    {
      return m_connection_role;
    }

  private:
    CppSQLite3DB m_ipc_database;
    CppSQLite3Statement m_begin_transaction_stmt;
    CppSQLite3Statement m_commit_transaction_stmt;

    CppSQLite3Statement m_insert_message_stmt;
    CppSQLite3Statement m_update_message_stmt;
    CppSQLite3Statement m_delete_old_messages_stmt;
    CppSQLite3Statement m_select_messages_stmt;
    ConnectionRole m_connection_role;

    std::string m_channel;
  };
    

  Petunia::Petunia(std::string &channel, ConnectionRole connection_role /* = Auto */)
    : m_channel(channel), m_mq_thread(nullptr)
  {
    Connect(connection_role);
  }

  void Petunia::Connect(ConnectionRole connection_role)
  {
    m_channel_path = GenerateChannelPath();
    m_ipc_medium = new IPCMedium(m_channel_path, connection_role);
    StartMQThread();
  }

  std::string Petunia::GenerateChannelPath()
  {
    std::string petunia_folder_path = GetPetuniaFolder();

    if (!OSUtils::FolderExists(petunia_folder_path))
    {
      if (!OSUtils::CreateFolder(petunia_folder_path))
      {
        return nullptr;
      }
    }

    return petunia_folder_path + m_channel + CHANNEL_FILE_EXTENSION;
  }

  Petunia::~Petunia()
  {
    TerminateMQThread();
    delete m_ipc_medium;
  }

  std::string Petunia::GetID()
  {
    return m_channel;
  }

  void Petunia::SendMessage(Message *message)
  {
    std::lock_guard<std::mutex> locked(m_send_lock);
    m_outbox_queue.push(message);
  }

  Message *Petunia::PollMessage()
  {
    Message *message = nullptr;
  
    if (!m_inbox_queue.empty())
    {
      message = m_inbox_queue.front();
      m_inbox_queue.pop();
    }

    return message;
  }

  bool Petunia::EnqueueReceivedMessages()
  {
    std::lock_guard<std::mutex> locked(m_receive_lock);
    return m_ipc_medium->EnqueueReceivedMessages(m_inbox_queue);
  }

  bool Petunia::SendEnqueuedMessages()
  {
    std::lock_guard<std::mutex> locked(m_send_lock);
    return m_ipc_medium->SendEnqueuedMessages(m_outbox_queue);
  }

  void Petunia::StartMQThread()
  {
    m_running = true;
    m_mq_thread = new std::thread([&]() { ThreadLoop(); });
  }

  void Petunia::ThreadLoop()
  {
    while (m_running)
    {
      bool should_sleep = !SendEnqueuedMessages();
      should_sleep = !EnqueueReceivedMessages() || should_sleep;
      if (should_sleep) {       
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      Distribute();
    }
  }

  void Petunia::TerminateMQThread()
  {
    m_running = false;
    m_mq_thread->join();
    delete m_mq_thread;
  }

  std::string Petunia::GetPetuniaFolder()
  {
    return OSUtils::GetTemporaryFolder() + CHANNEL_PATH_SUBFOLDER;
  }

  std::string Petunia::GetChannel()
  {
    return m_channel;
  }

  size_t Petunia::Distribute()
  {
    std::lock_guard<std::mutex> locked(m_receive_lock);
    size_t count = m_inbox_queue.size();
    while (!m_inbox_queue.empty()) {
      Message *message = m_inbox_queue.front();
      m_inbox_queue.pop();
      auto search = m_message_listeners.find(message->GetType());
      if (search != m_message_listeners.end()) {
        for (auto it = search->second->begin(); it != search->second->end(); ++it) {
          (*it)(message->GetText(), message->GetSize(), message->GetData());
        }
      }

      delete message;
    }
    
    return count;
  }

  size_t Petunia::AddListener(std::string& name, std::function<void(const char *text, unsigned long long size, const void *data)> listener_function)
  {
    std::list <std::function<void(const char *text, unsigned long long size, const void *data)>> *list = nullptr;
    auto search = m_message_listeners.find(name);
    if (search == m_message_listeners.end()) {
      list = new std::list <std::function<void(const char *text, unsigned long long size, const void *data)>>();
      m_message_listeners.insert(std::make_pair(name, list));
    }

    list->push_front(listener_function);

    return list->size();
  }

    void Petunia::RemoveListeners(std::string& name)
    {
      auto search = m_message_listeners.find(name);
      if (search != m_message_listeners.end()) {
        m_message_listeners.erase(search);
      }
    }

    void Petunia::RemovePromises(std::string& name)
    {

    }

    void Petunia::Clear()
    {
      ClearPromises();
      ClearListeners();      
    }

    void Petunia::ClearListeners()
    {
      m_message_listeners.clear();
    }

    void Petunia::ClearPromises()
    {

    }

} // namespace Petunia