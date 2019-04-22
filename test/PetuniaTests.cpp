#include <gtest/gtest.h>
#include <petunia/petunia.h>
#include <future>;

TEST(PetuniaTests, HelloTest) {
  EXPECT_TRUE(true); 
}

TEST(PetuniaTests, PetuniaMessageSend) {
  std::string channel = "test";
  std::string message_type = "test_message";
  std::promise<bool> promise;
  std::future<bool> future;

  Petunia::Petunia *server = new Petunia::Petunia(channel, Petunia::Server);
  Petunia::Petunia *client = new Petunia::Petunia(channel, Petunia::Client);
  server->SendMessage(new Petunia::Message(message_type.c_str(), "test", nullptr));
  client->AddListener(message_type, [&](const char *text, unsigned long long size, const void *data) {
    printf("Received: %s %llu 0x%p\n", text, size, data);
    promise.set_value(true);
  });

  future = promise.get_future();

  printf("Waiting...");
  future.wait();
  printf(" no more!\n");

  delete client;
  delete server;
}

