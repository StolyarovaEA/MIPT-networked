#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include "socket_tools.h"
#include <unistd.h>
#include <thread>


void keep_alive(int sfd, addrinfo r_addr)
{
  while(true)
  {
    std::string msg = std::to_string(std::rand());
    ssize_t res = sendto(sfd, msg.c_str(), msg.size(), 0, r_addr.ai_addr, r_addr.ai_addrlen);
    if (res == -1)
    {
      std::cout << strerror(errno) << std::endl;
    }
    sleep(5);
  }
}

void init(int sfd, addrinfo r_addr)
{
  srand(time(0));
  std::string msg = "init" + std::to_string(std::rand());
  ssize_t res = sendto(sfd, msg.c_str(), msg.size(), 0, r_addr.ai_addr, r_addr.ai_addrlen);
  if (res == -1)
  {
    std::cout << strerror(errno) << std::endl;
  }
}

void send_data(int sfd, addrinfo r_addr)
{
  while(true)
  {
    std::thread t_sender([&]() { 
    std::string input = "data"+std::to_string(std::rand());
    ssize_t res = sendto(sfd, input.c_str(), input.size(), 0, r_addr.ai_addr, r_addr.ai_addrlen);
    if (res == -1)
    {
      std::cout << strerror(errno) << std::endl;
    }
    printf("> %s\n", input.c_str());
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    t_sender.join();
    sleep(20);
  }
}

void get_data(int sfd)
{
  while(true)
  {
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(sfd, &readSet);

  timeval timeout = { 0, 100000 }; // 100 ms
  select(sfd + 1, &readSet, NULL, NULL, &timeout);

  if (FD_ISSET(sfd, &readSet))
    {
      constexpr size_t buf_size = 1000;

      static char buffer[buf_size];
      memset(buffer, 0, buf_size);

      ssize_t numBytes = recvfrom(sfd, buffer, buf_size - 1, 0, nullptr, nullptr);
      if (numBytes > 0)
        printf("%s\n", buffer); // assume that buffer is a string
    }
  }
}

int main(int argc, const char **argv)
{
  const char *port = "2022";
  const char *r_port = "2023";

  addrinfo resAddrInfo;
  int sfd = create_dgram_socket("localhost", port, &resAddrInfo);
  int r_sfd = create_dgram_socket(nullptr, r_port, nullptr);

  if (sfd == -1)
  {
    printf("Cannot create a socket\n");
    return 1;
  }
  if (r_sfd == -1)
  {
    printf("Cannot create a socket\n");
    return 1;
  }

  init(sfd, resAddrInfo);
  std::thread thread_1(keep_alive, sfd, resAddrInfo);
  std::thread thread_2(send_data, sfd, resAddrInfo);
  std::thread thread_3(get_data, r_sfd);

  thread_1.join();
  thread_2.join();
  thread_3.join();

  return 0;
}
