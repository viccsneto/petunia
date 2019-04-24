#include <gtest/gtest.h>
#include <petunia/petunia.h>
#include <future>

TEST(PetuniaTests, PetuniaMessageSend) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::string message_content = "content of the message";
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  Petunia::Petunia *server = new Petunia::Petunia(channel, Petunia::ConnectionRole::Server);
  Petunia::Petunia *client = new Petunia::Petunia(channel);
 
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

