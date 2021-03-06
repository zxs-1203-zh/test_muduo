#include "TimerId.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <stdio.h>

void runInThread()
{
  printf("runInThread(): pid = %d, tid = %d\n",
         getpid(), muduo::CurrentThread::tid());
}

int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), muduo::CurrentThread::tid());

  muduo::EventLoopThread loopThread;
  muduo::EventLoop* loop = loopThread.startLoop();
  loop->runInLoop(runInThread);
  loop->runAfter(2, runInThread);
  loop->quit();

  printf("exit main().\n");
}
