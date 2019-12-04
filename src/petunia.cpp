#include <assert.h>
#include <chrono>
#include <iostream>

#include <petunia/petunia.h>
#include <petunia/ipc_medium.h>
#include <petunia/message.h>
#include <petunia/osutils.h>

#define CHANNEL_PATH_SUBFOLDER "petunia/"

namespace Petunia
{       
  Petunia::Petunia(IPCMedium* medium)
    : m_ipc_medium(medium)
  {
    Connect();
  }

  void Petunia::Connect()
  {    
    StartMQThread();
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

  void Petunia::SendMessage(std::shared_ptr<Message> message)
  {
    std::lock_guard<std::mutex> locked(m_send_lock);
    m_outbox_queue.push(message);
    m_send_condition_variable.notify_all();
  }

  void Petunia::UpdateMessage(std::shared_ptr<Message> message)
  {
    message->SetOverwriteMode(true);
    SendMessage(message);
  }

  bool Petunia::EnqueueReceivedMessages()
  {
    return m_ipc_medium->ReceiveMessages(m_inbox_queue);
  }

  bool Petunia::SendEnqueuedMessages()
  {
    return m_ipc_medium->SendMessages(m_outbox_queue);
  }

  void Petunia::StartMQThread()
  {
    m_running = true;
    m_mq_receive_thread = new std::thread([&]() { ReceiveThreadLoop(); });
    m_mq_send_thread = new std::thread([&]() { SendThreadLoop(); });
  }

  void Petunia::SendThreadLoop()
  {
    while (m_running)
    {
      std::unique_lock<std::mutex> locked(m_send_lock);
      m_send_condition_variable.wait(locked, [&] {return m_outbox_queue.size() > 0 || !m_running;});
      if (m_running) {
        SendEnqueuedMessages();
      }
    }
  }

  void Petunia::ReceiveThreadLoop()
  {
    while (m_running)
    {
      std::unique_lock<std::mutex> locked(m_receive_lock);
      if (!EnqueueReceivedMessages()) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
      } else {
        Distribute();
      }
    }
  }

  void Petunia::TerminateMQThread()
  {
    m_running = false;
    m_send_condition_variable.notify_all();
    m_mq_receive_thread->join();
    m_mq_send_thread->join();
    delete m_mq_receive_thread;
    delete m_mq_send_thread;
  }

  std::string Petunia::GetPetuniaFolder()
  {
    std::string petunia_folder_path = OSUtils::GetTemporaryFolder() + CHANNEL_PATH_SUBFOLDER;
    if (!OSUtils::FolderExists(petunia_folder_path))
    {
      if (!OSUtils::CreateFolder(petunia_folder_path))
      {
        return nullptr;
      }
    }
    return petunia_folder_path;
  }

  std::string Petunia::GetChannel()
  {
    return m_channel;
  }

  size_t Petunia::Distribute()
  {
    size_t count = m_inbox_queue.size();
    while (!m_inbox_queue.empty()) {
      std::shared_ptr<Message> message = m_inbox_queue.front();
      m_inbox_queue.pop();
      auto search = m_message_listeners.find(message->GetType());
      if (search != m_message_listeners.end()) {
        for (auto it = search->second->begin(); it != search->second->end(); ++it) {
          (*it)(message);
        }
      }
    }
    
    return count;
  }

  size_t Petunia::AddListener(std::string& name, std::function<void(std::shared_ptr<Message> message)> listener_function)
  {
    std::list <std::function<void(std::shared_ptr<Message> message)>> *list = nullptr;
    auto search = m_message_listeners.find(name);
    if (search == m_message_listeners.end()) {
      list = new std::list <std::function<void(std::shared_ptr<Message> message)>>();
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