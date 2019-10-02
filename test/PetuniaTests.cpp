#include <gtest/gtest.h>
#include <petunia/petunia.h>
#include <petunia/ipc_medium_default.h>
#include <future>


TEST(PetuniaTests, PetuniaMessageSend) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::string message_content = "content of the message";
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  Petunia::Petunia *server = new Petunia::Petunia(new Petunia::IPCMediumDefault(channel, Petunia::ConnectionRole::Server));
  Petunia::Petunia *client = new Petunia::Petunia(new Petunia::IPCMediumDefault(channel));
 
  client->AddListener(message_type, [&](const Petunia::Message &message) {
    EXPECT_STREQ(message_content.c_str(), message.GetText());
    client->SendMessage(new Petunia::Message(message_type, message_content));
  });

  server->AddListener(message_type, [&](const Petunia::Message &message) {
    EXPECT_STREQ(message_content.c_str(), message.GetText());
    printf("No more!!!\n");
    printf("Received: %s %llu 0x%p\n", message.GetText(), message.GetDataSize(), message.GetData());
    promise.set_value(true);
  });

  server->SendMessage(new Petunia::Message(message_type, message_content));
  

  printf("Waiting...");
  future.wait();
  
  delete client;
  delete server;
}

TEST(PetuniaTests, PetuniaManyMessagesSend) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::string message_content = "content of the message";
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  int count = 1024;
  size_t message_size = 1024 * 1024 * 2; // 2MB
  
  if (message_size < message_content.size()) {
    message_content.resize(message_size);
  }
  else {
    message_content.resize(message_size, 'A');
  }


  Petunia::Petunia *server = new Petunia::Petunia(new Petunia::IPCMediumDefault(channel, Petunia::ConnectionRole::Server));
  Petunia::Petunia *client = new Petunia::Petunia(new Petunia::IPCMediumDefault(channel));
  
  client->AddListener(message_type, [&](const Petunia::Message &message) {
    EXPECT_STREQ(message_content.c_str(), message.GetText());
    client->SendMessage(new Petunia::Message(message_type, message_content));
  });

  server->AddListener(message_type, [&](const Petunia::Message &message) {
    EXPECT_STREQ(message_content.c_str(), message.GetText());
    if (--count <= 0) {
      printf("No more!!!\n");
      promise.set_value(true);
    } else {
      server->SendMessage(new Petunia::Message(message_type, message_content));
    }
  });

  server->SendMessage(new Petunia::Message(message_type, message_content));

  printf("Waiting...");
  future.wait();
  
  delete client;
  delete server;
}

