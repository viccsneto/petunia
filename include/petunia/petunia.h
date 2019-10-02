#pragma once
#include <mutex>
#include <thread>
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
    void              SendMessage(Message *msg);
    void              UpdateMessage(Message *msg);
    std::string       GetChannel();
    size_t            Distribute();
    size_t            AddListener(std::string& name, std::function<void(const Message &message)> listener_function);
    void              RemoveListeners(std::string& name);
    void              RemovePromises(std::string& name);
    void              Clear();
    void              ClearListeners();
    void              ClearPromises();
    static            std::string GetPetuniaFolder();
  private:
    Message     *PollMessage();
    bool         EnqueueReceivedMessages();
    bool         SendEnqueuedMessages();
    void         StartMQThread();
    void         ThreadLoop();
    void         TerminateMQThread();
    
    void Connect();

  private:
    IPCMedium            *m_ipc_medium;
    std::string           m_channel;
    std::string           m_channel_path;
    std::queue<Message *> m_inbox_queue;
    std::queue<Message *> m_outbox_queue;
    std::mutex            m_send_lock;
    std::mutex            m_receive_lock;
    std::thread          *m_mq_thread;
    bool                  m_running;
    std::unordered_map<std::string, std::list<std::function<void(const Message &message)>> *> m_message_listeners;
  };
} // namespace Petunia
