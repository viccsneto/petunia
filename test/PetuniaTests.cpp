#include <gtest/gtest.h>
#include <petunia/petunia.h>
#include <future>;

TEST(PetuniaTests, PetuniaMessageSend) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::promise<bool> promise;
  std::future<bool> future = promise.get_future();

  Petunia::Petunia *server = new Petunia::Petunia(channel, Petunia::ConnectionRole::Server);
  Petunia::Petunia *client = new Petunia::Petunia(channel);
 
  client->AddListener(message_type, [&](const Petunia::Message &message) {
    client->SendMessage(new Petunia::Message(message_type.c_str(), "test", nullptr));
  });

  server->AddListener(message_type, [&](const Petunia::Message &message) {
    printf("No more!!!\n");
    printf("Received: %s %llu 0x%p\n", message.GetText(), message.GetDataSize(), message.GetData());
    promise.set_value(true);
  });

  server->SendMessage(new Petunia::Message(message_type.c_str(), "test", nullptr));
  

  printf("Waiting...");
  future.wait();
  
  delete client;
  delete server;
}

