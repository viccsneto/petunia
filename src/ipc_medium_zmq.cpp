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
        m_socket = new zmq::socket_t(m_context, ZMQ_REP);
      }
      else {
        m_socket = new zmq::socket_t(m_context, ZMQ_REQ);
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
      zmq::message_t message_type;
      m_socket->recv(&message_type);

      zmq::message_t message_body;
      m_socket->recv(&message_body);

      inbox_queue.push(std::make_shared<Petunia::Message>(std::string(message_type.data()),)

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