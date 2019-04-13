#pragma once
#include <mutex>
#include <thread>
#include <string>
#include <queue>

#include "message.h"
#include "message_handler.h"

namespace Petunia
{  
  class IPCMedium;

  class Petunia 
  {
  public:
    explicit Petunia(std::string channel);

    void Connect();

    ~Petunia();
    std::string       GetID();
    void              SendMessage(Message *msg);
    Message *PollMessage();
    std::string       GetChannel();

  private:
    std::string GenerateChannelPath();
    bool        EnqueueReceivedMessages();
    bool        SendEnqueuedMessages();
    void        StartMQThread();
    void        ThreadLoop();
    void        TerminateMQThread();
    std::string GetPetuniaFolder();

  private:
    IPCMedium             *m_ipc_medium;
    std::string           m_channel;
    std::string           m_channel_path;   
    std::queue<Message *> m_inbox_queue;
    std::queue<Message *> m_outbox_queue;
    std::mutex            m_send_lock;
    std::mutex            m_receive_lock;
    std::thread *         m_mq_thread;
    bool                  m_running;
  };
} // namespace Petunia
