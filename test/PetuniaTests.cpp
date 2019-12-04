#include <gtest/gtest.h>
#include <petunia/petunia.h>
#include <petunia/ipc_medium_default.h>
#include <petunia/ipc_medium_inprocess.h>
#include <petunia/ipc_medium_nanomsg.h>
#include <future>

// Put the name and the underscore together to disable the test:
#define DISABLED_EXAMPLE_TEST_
// Separate the underscore from the name to enable the test:
#define ENABLED_EXAMPLE_TEST _

#define MESSAGE_SEND_INPROCESS_
#define MESSAGE_SEND_NANOMSG_
#define MESSAGE_SEND_DEFAULT_
#define MESSAGE_MANY_SEND_INPROCESS_
#define MESSAGE_MANY_SEND_NANOMSG _
#define MESSAGE_MANY_SEND_DEFAULT_




#ifdef MESSAGE_SEND_INPROCESS
TEST(PetuniaTests, PetuniaMessageSendInProcess) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::shared_ptr<std::string> message_content = std::make_shared<std::string>("content of the message");
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  Petunia::Petunia *server = new Petunia::Petunia(new Petunia::IPCMediumInprocess(channel, Petunia::ConnectionRole::Server));
  Petunia::Petunia *client = new Petunia::Petunia(new Petunia::IPCMediumInprocess(channel, Petunia::ConnectionRole::Client));

  client->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), message->GetData()->c_str());
    client->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));
    });

  server->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), message->GetData()->c_str());
    printf("No more!!!\n");
    printf("Received: %s %llu 0x%p\n", message->GetData()->c_str(), message->GetDataSize(), message->GetData().get());
    promise.set_value(true);
    });

  server->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));


  printf("Waiting...");
  future.wait();

  delete client;
  delete server;
}
#endif

#ifdef MESSAGE_SEND_NANOMSG           
TEST(PetuniaTests, PetuniaMessageSendNanomsg) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::shared_ptr<std::string> message_content = std::make_shared<std::string>("content of the message");
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  Petunia::Petunia *server = new Petunia::Petunia(new Petunia::IPCMediumNanomsg(channel, Petunia::ConnectionRole::Server));
  Petunia::Petunia *client = new Petunia::Petunia(new Petunia::IPCMediumNanomsg(channel, Petunia::ConnectionRole::Client));

  client->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    client->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));
    });

  server->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    printf("No more!!!\n");
    printf("Received: %s %llu 0x%p\n", message->GetData()->c_str() , message->GetDataSize(), message->GetData().get());
    promise.set_value(true);
    });

  server->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));


  printf("Waiting...");
  future.wait();

  delete client;
  delete server;
}
#endif

#ifdef MESSAGE_SEND_DEFAULT
TEST(PetuniaTests, PetuniaMessageSendDefault) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::shared_ptr<std::string> message_content = std::make_shared<std::string>("content of the message");
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  Petunia::Petunia *server = new Petunia::Petunia(new Petunia::IPCMediumDefault(channel, Petunia::ConnectionRole::Server));
  Petunia::Petunia *client = new Petunia::Petunia(new Petunia::IPCMediumDefault(channel, Petunia::ConnectionRole::Client));

  client->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    client->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));
    });

  server->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    printf("No more!!!\n");
    printf("Received: %s %llu 0x%p\n", message->GetData()->c_str(), message->GetDataSize(), message->GetData());
    promise.set_value(true);
    });

  server->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));


  printf("Waiting...");
  future.wait();

  delete client;
  delete server;
}
#endif

#ifdef MESSAGE_MANY_SEND_INPROCESS
TEST(PetuniaTests, PetuniaManyMessagesSendInprocess) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::shared_ptr<std::string> message_content = std::make_shared<std::string>("content of the message");
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  int count = 1024;
  size_t message_size = 1024 * 1024 * 2; // 2MB

  if (message_size < message_content->size()) {
    message_content->resize(message_size);
  }
  else {
    message_content->resize(message_size, 'A');
  }


  Petunia::Petunia *server = new Petunia::Petunia(new Petunia::IPCMediumInprocess(channel, Petunia::ConnectionRole::Server));
  Petunia::Petunia *client = new Petunia::Petunia(new Petunia::IPCMediumInprocess(channel, Petunia::ConnectionRole::Client));

  client->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    client->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));
    });

  server->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    if (--count <= 0) {
      printf("No more!!!\n");
      promise.set_value(true);
    }
    else {
      server->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));
    }
    });

  server->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));

  printf("Waiting...");
  future.wait();

  delete client;
  delete server;
}
#endif

#ifdef MESSAGE_MANY_SEND_NANOMSG
TEST(PetuniaTests, PetuniaManyMessagesSendNanomsg) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::shared_ptr<std::string> message_content = std::make_shared<std::string>("content of the message");
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  int count = 1024;
  size_t message_size = 1024 * 1 * 2; // 2MB

  if (message_size < message_content->size()) {
    message_content->resize(message_size);
  }
  else {
    message_content->resize(message_size, 'A');
  }


  Petunia::Petunia *server = new Petunia::Petunia(new Petunia::IPCMediumNanomsg(channel, Petunia::ConnectionRole::Server));
  Petunia::Petunia *client = new Petunia::Petunia(new Petunia::IPCMediumNanomsg(channel, Petunia::ConnectionRole::Client));

  client->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    client->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));
    });

  server->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    if (--count <= 0) {
      printf("No more!!!\n");
      promise.set_value(true);
    }
    else {
      server->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));
    }
    });

  server->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));

  printf("Waiting...");
  future.wait();

  delete client;
  delete server;
}

#endif

#ifdef MESSAGE_MANY_SEND_DEFAULT
TEST(PetuniaTests, PetuniaManyMessagesSendDefault) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::shared_ptr<std::string> message_content = std::make_shared<std::string>("content of the message");
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  int count = 1024;
  size_t message_size = 1024 * 1024 * 2; // 2MB
  
  if (message_size < message_content->size()) {
    message_content->resize(message_size);
  }
  else {
    message_content->resize(message_size, 'A');
  }


  Petunia::Petunia *server = new Petunia::Petunia(new Petunia::IPCMediumDefault(channel, Petunia::ConnectionRole::Server));
  Petunia::Petunia *client = new Petunia::Petunia(new Petunia::IPCMediumDefault(channel, Petunia::ConnectionRole::Client));
  
  client->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    client->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));
  });

  server->AddListener(message_type, [&](std::shared_ptr<Petunia::Message> message) {
    EXPECT_STREQ(message_content->c_str(), (const char *)message->GetData().get()->c_str());
    if (--count <= 0) {
      printf("No more!!!\n");
      promise.set_value(true);
    } else {
      server->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));
    }
  });

  server->SendMessage(std::make_shared<Petunia::Message>(message_type, message_content));

  printf("Waiting...");
  future.wait();
  
  delete client;
  delete server;
}
#endif