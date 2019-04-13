#include "sqlite3/CppSQLite3.h"

#include <assert.h>
#include <chrono>
#include <iostream>
#include <list>

#include <petunia/petunia.h>
#include <petunia/message.h>
#include <petunia/osutils.h>

#define CHANNEL_PATH_SUBFOLDER "petunia/"
#define CHANNEL_FILE_EXTENSION ".ipc.db"

namespace Petunia
{
  class IPCMedium
  {
  public:
    IPCMedium(std::string &channel)
    {
      InitializeDatabase(channel);
        

    }

    void InitializeDatabase(std::string &channel)
    {
      m_ipc_database.open(channel.c_str());

      m_ipc_database.execDML("PRAGMA synchronous = OFF");
      m_ipc_database.execDML("PRAGMA journal_mode = OFF");
      m_ipc_database.execDML("PRAGMA mmap_size=44194304");
      m_ipc_database.execDML("PRAGMA busy_timeout=30000");
      
      m_server_mode = !m_ipc_database.tableExists("server_messages");

      if (IsServerMode()) {
        m_ipc_database.execDML("CREATE TABLE server_messages(type TEXT, size NUMBER, blob_message BLOB, text_message TEXT, row_id INTEGER PRIMARY KEY)");
        m_ipc_database.execDML("CREATE TABLE client_messages(type TEXT, size NUMBER, blob_message BLOB, text_message TEXT, row_id INTEGER PRIMARY KEY)");
        m_ipc_database.execDML("CREATE INDEX idx_server_messages_type ON server_messages(type)");
        m_ipc_database.execDML("CREATE INDEX idx_client_messages_type ON client_messages(type)");
      }

      
      CreateStatements();
      
    }

    void CreateStatements()
    {
      m_begin_transaction_stmt = m_ipc_database.compileStatement("BEGIN TRANSACTION");
      m_commit_transaction_stmt = m_ipc_database.compileStatement("COMMIT");

      if (IsServerMode()) {
        m_insert_message_stmt = m_ipc_database.compileStatement("INSERT INTO server_messages "
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

        m_delete_messages_stmt = m_ipc_database.compileStatement("DELETE FROM client_messages WHERE row_id <= @row_id");
        m_select_messages_stmt = m_ipc_database.compileStatement("SELECT * FROM client_messages");
      } else {
        m_insert_message_stmt = m_ipc_database.compileStatement("INSERT INTO client_messages "
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

        m_delete_messages_stmt = m_ipc_database.compileStatement("DELETE FROM server_messages WHERE row_id <= @row_id");
        m_select_messages_stmt = m_ipc_database.compileStatement("SELECT * FROM server_messages");
      }      
    }
  

    ~IPCMedium()
    {
      m_ipc_database.close();
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
      m_select_messages_stmt.reset();
      CppSQLite3Query q = m_select_messages_stmt.execQuery();
      unsigned long long id_to_delete = 0;

      while (!q.eof()) {
        unsigned long long row_id = q.getInt64Field("row_id");

        if (row_id > id_to_delete) {
          id_to_delete = row_id;
        }

        size_t size = q.getSizeTField("size");
        void *buffer = malloc(size);
        if (!buffer) {
          printf("PANIC!\n");
          assert(false);
          exit(-1);
        }

        int nSize;
        const unsigned char *message = q.getBlobField("blob_message", nSize);
        assert(size == nSize);
        memcpy(buffer, message, size);
        Message *electron_msg = new Message(std::string(q.getStringField("type")), std::string(q.getStringField("text_message")), size, buffer);
        inbox_queue.push(electron_msg);
        q.nextRow();
      }

      if (id_to_delete > 0) {
        BeginTransaction();
        m_delete_messages_stmt.reset();
        m_delete_messages_stmt.bindSizeT("@row_id", id_to_delete);
        m_delete_messages_stmt.execDML();
        Commit();
        return true;
      }

      return false;
    }


    bool SendEnqueuedMessages(std::queue<Message *> &outbox_queue)
    {
      if (!outbox_queue.empty()) {

        BeginTransaction();
        while (!outbox_queue.empty()) {
          Message *msg = outbox_queue.front();
          m_insert_message_stmt.reset();
          m_insert_message_stmt.bind("@type", msg->GetType().c_str());
          m_insert_message_stmt.bindSizeT("@size", msg->GetSize());
          m_insert_message_stmt.bind("@text_message", (const unsigned char *)msg->GetText(), msg->GetTextSize());
          m_insert_message_stmt.bind("@blob_message", (const unsigned char *)msg->GetData(), msg->GetSize());
          m_insert_message_stmt.execQuery();
          outbox_queue.pop();
          delete msg;
        }
        Commit();
        return true;
      }

      return false;
    }

    bool IsServerMode()
    {
      return m_server_mode;
    }
    
  private:
    CppSQLite3DB m_ipc_database;
    CppSQLite3Statement m_insert_message_stmt;
    CppSQLite3Statement m_begin_transaction_stmt;
    CppSQLite3Statement m_commit_transaction_stmt;
    CppSQLite3Statement m_update_message_stmt;
    CppSQLite3Statement m_delete_messages_stmt;
    CppSQLite3Statement m_select_messages_stmt;
    bool m_server_mode;
  };

  

  Petunia::Petunia(std::string channel)
  : m_channel(channel)
  , m_mq_thread(nullptr)
  {
    Connect();
  }

  void Petunia::Connect()
  {
    m_channel_path = GenerateChannelPath();
    m_ipc_medium = new IPCMedium(m_channel_path);
    StartMQThread();
  }

  std::string Petunia::GenerateChannelPath()
  {
    std::string petunia_folder_path = GetPetuniaFolder();

    if (!OSUtils::FolderExists(petunia_folder_path)) {
      if (!OSUtils::CreateFolder(petunia_folder_path)) {
        return nullptr;
      }
    }

    return petunia_folder_path + m_channel + CHANNEL_FILE_EXTENSION;
  }

  Petunia::~Petunia()
  {
    TerminateMQThread();
    delete m_ipc_medium;
    remove(m_channel_path.c_str());
  }

  std::string Petunia::GetID()
  {
    return m_channel;
  }

  void Petunia::SendMessage(Message *msg)
  {
    std::lock_guard<std::mutex> locked(m_send_lock);
    m_outbox_queue.push(msg);
  }

  Message *Petunia::PollMessage()
  {
    Message *electron_msg = nullptr;
    std::lock_guard<std::mutex> locked(m_receive_lock);

    if (!m_inbox_queue.empty()) {
      electron_msg = m_inbox_queue.front();
      m_inbox_queue.pop();
    }

    return electron_msg;
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
    while (m_running) {
      bool should_sleep = !SendEnqueuedMessages();
      should_sleep = !EnqueueReceivedMessages() || should_sleep;

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
} // namespace Petunia