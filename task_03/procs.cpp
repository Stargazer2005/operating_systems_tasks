// Process, lasting for 10 seconds
#include <iostream>
#include <unistd.h>

int main() {
  std::cout << "Process" << std::endl;
  sleep(10);
  return 0;
}
