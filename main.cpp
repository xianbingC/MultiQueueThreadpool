#include "MultiQueueThreadPool.h"
#include <iostream>
#include <unistd.h>

void task(unsigned i) 
{
  std::cout << "task " << i << "done" << std::endl;
}

int main()
{
  MultiQueueThreadPool tp(5);

  unsigned i = 0;
  while(1) {
    sched_yield();
    tp.enqueue(i, task, i);
    i++;
    if (i % 100 == 0) {
      sleep(1);
    }
  }
  return 0;
}
