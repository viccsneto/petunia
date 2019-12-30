#pragma once
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>
#include <queue>
#include <unordered_map>
#include <functional>
#include <list>

#include "message.h"
#include "types.h"
#include "ipc_medium.h"

namespace Petunia
{
  class Petunia
  {
  public:
    explicit Petunia(IPCMedium* medium);

    ~Petunia();
    std::string       GetID();
    void              SendMessage(std::shared_ptr<Message> msg);
    void              UpdateMessage(std::shared_ptr<Message> msg);
    const std::string       GetChannel() const;
    size_t            Distribute();
    size_t            AddListener(std::string name, std::function<void(std::shared_ptr<Message> message)> listener_function);
    bool              RemoveListeners(std::string name);
    void              RemovePromises(std::string name);
    void              Clear();
    void              ClearListeners();
    void              ClearPromises();
    static            std::string GetPetuniaFolder();
  private:
    bool         EnqueueReceivedMessages();
    bool         SendEnqueuedMessages();
    void         StartMQThread();
    void         SendThreadLoop();
    void         ReceiveThreadLoop();
    void         TerminateMQThread();
    
    void Connect();

  private:
    IPCMedium               *m_ipc_medium;
    std::string              m_channel;
    std::string              m_channel_path;
    std::queue<std::shared_ptr<Message>>    m_inbox_queue;
    std::queue<std::shared_ptr<Message>>    m_outbox_queue;
    std::mutex               m_send_lock;
    std::mutex               m_receive_lock;
    std::thread             *m_mq_receive_thread;
    std::thread             *m_mq_send_thread;
    std::condition_variable  m_send_condition_variable;
    bool                     m_running;
    std::unordered_map<std::string, std::list<std::function<void(std::shared_ptr<Message> message)>> *> m_message_listeners;
  };
} // namespace Petunia
