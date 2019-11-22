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
      m_nano_socket = std::make_shared<nn::socket>(AF_SP, NN_PAIR);
      if (connection_role == ConnectionRole::Server) {
        m_nano_socket->bind((std::string("ipc:///") + channel).c_str());
      }
      else {
        m_nano_socket->connect((std::string("ipc:///") + channel).c_str());
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
          m_nano_socket->send(message.get()->GetData().get(), message->GetDataSize(), 0);
        
          outbox_queue.pop();
        }
        
        return true;
      }

      return false;
    }
    
    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue)
    {
      bool result = false;
      char *buffer = nullptr;
      while (int rc = m_nano_socket->recv(&buffer, 0, 0) > 0) {        
        std::string message_type(buffer);
        free(buffer);
        if (m_nano_socket->recv(&buffer, 0, 0) > 0) {
          std::shared_ptr<std::string> data = std::make_shared<std::string>();
          data->reserve(rc);
          data->assign(buffer);
          inbox_queue.push(std::make_shared<Message>(message_type, data));
        }
        result = true;
      }

      return result;
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