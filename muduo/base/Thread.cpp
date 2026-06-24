// Thread.cpp
#include "Thread.h"
#include "CurrentThread.h"
#include <cstdio>
#include <semaphore.h>

using std::move;
using std::thread;
using std::shared_ptr;
using std::make_shared;

std::atomic<int> Thread::numCreated_ { 0 };

Thread::Thread(ThreadFunc f, const std::string& name)
    : func_(move(f)), name_(name)
{
    int n = ++numCreated_;

    // 创建默认的线程名
    if (name_.empty()) {
        char buf[32] = { 0 };
        snprintf(buf, sizeof(buf), "Thread%d", n);
        name_ = buf;
    }
}

Thread::~Thread() {
    if (start_ && !joined_) {   
        thread_->detach();
    }
}

void Thread::start() {
    sem_t sem;
    sem_init(&sem, 0, 0);
    // 开启线程
    auto thr = make_shared<thread>([&]() {
        tid_ = CurrentThread::tid();
        sem_post(&sem);  // 获取 tid 后 post
        
        func_();
    });

    start_ = true;
    thread_ = thr;

    // 保证在 start() 返回前获取 tid，否则导致未定义行为
    sem_wait(&sem);   
    sem_destroy(&sem);
}

void Thread::join() {
    joined_ = true;
    thread_->join();
}