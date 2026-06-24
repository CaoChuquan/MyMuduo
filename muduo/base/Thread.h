// Thread.h
#pragma once

#include "noncopyable.h"
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <atomic>

class Thread : noncopyable {
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc f, const std::string& name = "");
    ~Thread();

    void start();
    void join();

    bool started() const { return start_; }
    pid_t tid() const { return tid_; }
    const std::string name() const { return name_; }
    static int numCreate() { return numCreated_; }
private:
    // void setDefaultName();

    bool start_{ false };
    bool joined_{ false };
    std::shared_ptr<std::thread> thread_;
    pid_t tid_{ 0 };
    ThreadFunc func_;
    std::string name_;
    static std::atomic<int> numCreated_;
};