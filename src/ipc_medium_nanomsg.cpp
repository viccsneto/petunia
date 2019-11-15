#include <petunia/ipc_medium_nanomsg.h>
#include <assert.h>
#include <petunia/osutils.h>
#include <nanomsg/nn.hpp>
#include <nanomsg/pair.h>



namespace Petunia {
  class IPCInternalMedium : public IPCMedium
  {
  public:
    IPCInternalMedium(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/)
      :IPCMedium(channel, connection_role)
    {      
      m_nano_socket = std::make_unique<nn::socket>(AF_SP, NN_PAIR);
      if (connection_role == ConnectionRole::Server) {
        m_nano_socket->bind((std::string("ipc://") + channel).c_str());
      }
      else {
        m_nano_socket->connect((std::string("ipc://") + channel).c_str());
      }
    }
    
    ~IPCInternalMedium()
    {
    
    }

    bool SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue)
    {     
      if (!outbox_queue.empty()) {       
        while (!outbox_queue.empty()) {
          std::shared_ptr<Message> message = outbox_queue.front();
          
          m_nano_socket->send((const void *)message.get()->GetType(), strlen(message.get()->GetType()), 0);
          m_nano_socket->send((const void *)message.get()->GetText()->c_str(), message->GetText()->size(), 0);
        
          outbox_queue.pop();
        }
        
        return true;
      }

      return false;
    }
    
    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue)
    {
      bool result = false;
     
      while (int rc = m_nano_socket->recv(nullptr, 0, 0) > 0) {
        std::string message_type;
        message_type.reserve(rc);
        rc = m_nano_socket->recv((void *)message_type.data(), rc, 0);
        rc = m_nano_socket->recv(nullptr, 0, 0);
        std::shared_ptr<std::string> data = std::make_shared<std::string>();
        data->reserve(rc);
        rc = m_nano_socket->recv((void *)data->data(), rc, 0);
        
        inbox_queue.push(std::make_shared<Message>(message_type, data));
        
        result = true;
      }

      return result;
    }
  private:
    std::unique_ptr<nn::socket> m_nano_socket;
  };
  
  IPCMediumNanomsg::IPCMediumNanomsg(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/)
    :IPCMedium(channel, connection_role)
  {
    m_internal_medium = std::make_unique<IPCInternalMedium>(channel, connection_role);
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