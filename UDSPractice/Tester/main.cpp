#include <stdio.h>

#include <cstdint>
#include <iostream>
#include <vector>

#include "doip_client.h"
#include "udppipe.h"

std::vector<uint8_t> send_buffer;

int main() {
  UDP_PIPE_InitB();
  char c;
  // send_buffer.resize(1);
  std::cout << "Start Send Datas to Server, Press 's' to Stop, 'b' to Break\n";

  while (1) {
    std::cout << "Intput StreamDatas to Send to Server\n";
    if ((c = std::cin.get()) == 's') {
      break;
    }
    send_buffer.push_back(c - 48);// push_back自动申请内存，这样vector的size就可以逐步增加，不会越界,
                                 //下标是修改不是插入，如果要用下标，需要保证初始化时候有足够的元素

    while ((c = std::cin.get()) != '\n' && c != EOF) {
      send_buffer.push_back(c - 48);
    }

    if (send_buffer.size() < 1) {
      std::cout << "DataBuffer out of range!";
      return 0;
    } else {
      UDP_PIPE_Send(&send_buffer[0], (uint16_t)send_buffer.size());
      send_buffer.clear();
    }
  }
  if (c == 's') {
    std::cout << "Send Data stopped !!\n";
  }
  return 0;
}
