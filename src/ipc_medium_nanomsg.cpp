#include <petunia/ipc_medium_nanomsg.h>
#include <assert.h>
#include <petunia/osutils.h>
#include <nanomsg/nn.hpp>
#include <nanomsg/pair.h>
#include <iostream>


namespace Petunia {
  class IPCInternalMedium : public IPCMedium
  {
  public:
    IPCInternalMedium(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/)
      :IPCMedium(channel, connection_role)
    {      
#ifdef _WIN32
      std::string channel_prefix = "ipc:///";
#else 
      std::string channel_prefix = "ipc:///tmp/";
#endif
      m_nano_socket = std::make_shared<nn::socket>(AF_SP, NN_PAIR);
      int max_size = -1;
      m_nano_socket->setsockopt(NN_SOL_SOCKET, NN_RCVMAXSIZE, &max_size, sizeof(max_size));
      if (connection_role == ConnectionRole::Server) {
        m_nano_socket->bind((channel_prefix + channel).c_str());
      }
      else {
        m_nano_socket->connect((channel_prefix + channel).c_str());
      }
    }
    
    ~IPCInternalMedium()
    {
      std::cout << "shutdown " << m_channel << " : " << (m_connection_role == ConnectionRole::Server ? "Server" : "Client");
    }

    bool SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue)
    {     
      if (!outbox_queue.empty()) {
        while (!outbox_queue.empty()) {
          std::shared_ptr<Message> message = outbox_queue.front();

          m_nano_socket->send(message.get()->GetType(), strlen(message.get()->GetType()) + 1, 0);
          m_nano_socket->send(message.get()->GetData().get()->c_str(), message->GetDataSize(), 0);

          outbox_queue.pop();
        }
        return true;
      }      

      return false;
    }
    
    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue)
    {
      char *buffer = nullptr;
      int rc;
      bool received = false;
      while ((rc = m_nano_socket->recv(&buffer, NN_MSG, NN_DONTWAIT)) > 0) {        
        std::string message_type(buffer);
        nn_freemsg(buffer);
        buffer = nullptr;
        if ((rc = m_nano_socket->recv(&buffer, NN_MSG, 0)) > 0) {
          std::shared_ptr<std::string> data = std::make_shared<std::string>();
          data->resize(rc);
          memcpy((char *)data->data(), buffer,  rc);
          nn_freemsg(buffer);
          inbox_queue.push(std::make_shared<Message>(message_type, data));
          received = true;
        }
      }
      return received;
    }
  private:
    std::shared_ptr<nn::socket> m_nano_socket;
  };
  
  IPCMediumNanomsg::IPCMediumNanomsg(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/)
    :IPCMedium(channel, connection_role)
  {
    m_internal_medium = std::make_shared<IPCInternalMedium>(channel, connection_role);
  }

  IPCMediumNanomsg::~IPCMediumNanomsg()
  {
    
  }

  bool IPCMediumNanomsg::SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue)
  {
    return m_internal_medium->SendMessages(outbox_queue);
  }


  bool IPCMediumNanomsg::ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue)
  {
    return m_internal_medium->ReceiveMessages(inbox_queue);
  }
}