#include <petunia/ipc_medium_zmq.h>
#include <assert.h>
#include <petunia/osutils.h>
#include <zmq.hpp>
#include <iostream>


namespace Petunia {
  class IPCInternalMedium : public IPCMedium
  {
  public:
    IPCInternalMedium(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/)
      :IPCMedium(channel, connection_role)
    {      
      std::string channel_prefix = "ipc:///tmp/";

      m_context = new zmq::context_t(1);      
      if (connection_role == ConnectionRole::Server) {
        m_socket = new zmq::socket_t(*m_context, ZMQ_REP);
      }
      else {
        m_socket = new zmq::socket_t(*m_context, ZMQ_REQ);
      }

      m_socket->bind((channel_prefix + channel).c_str());
    }
    
    ~IPCInternalMedium()
    {
      delete m_socket;
      delete m_context;
    }

    bool SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue)
    {     
      if (!outbox_queue.empty()) {
        while (!outbox_queue.empty()) {
          std::shared_ptr<Message> message = outbox_queue.front();
          
          size_t msg_type_size = strlen(message->GetType());
          zmq::message_t message_type(msg_type_size);
          memcpy(message_type.data(), message->GetType(), msg_type_size);
          m_socket->send(message_type);

          zmq::message_t message_body(message->GetDataSize());

          memcpy(message_body.data(), message->GetData()->data(), message->GetDataSize());
          m_socket->send(message_body);

          outbox_queue.pop();
        }
        return true;
      }      

      return false;
    }
    
    bool ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue)
    {
      zmq::message_t zmq_message_type;
      if (m_socket->recv(&zmq_message_type, ZMQ_NOBLOCK)) {
        zmq::message_t zmq_message_body;
        m_socket->recv(&zmq_message_body);

        std::string message_type((const char *)zmq_message_type.data());
        std::shared_ptr<std::string> message_data = std::make_shared<std::string>();
        message_data->resize(zmq_message_body.size());
        memcpy((char *)message_data->data(), zmq_message_body.data(), zmq_message_body.size());
        inbox_queue.push(std::make_shared<Message>(message_type,message_data));

        return true;
      }

      return false;
    }
  private:
    zmq::context_t *m_context;
    zmq::socket_t *m_socket;
  };
  
  IPCMediumZMQ::IPCMediumZMQ(std::string &channel, ConnectionRole connection_role /*= ConnectionRole::Auto*/)
    :IPCMedium(channel, connection_role)
  {
    m_internal_medium = std::make_shared<IPCInternalMedium>(channel, connection_role);
  }

  IPCMediumZMQ::~IPCMediumZMQ()
  {
    
  }

  bool IPCMediumZMQ::SendMessages(std::queue<std::shared_ptr<Message>> &outbox_queue)
  {
    return m_internal_medium->SendMessages(outbox_queue);
  }


  bool IPCMediumZMQ::ReceiveMessages(std::queue<std::shared_ptr<Message>> &inbox_queue)
  {
    return m_internal_medium->ReceiveMessages(inbox_queue);
  }
}