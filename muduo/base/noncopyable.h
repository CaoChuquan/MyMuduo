#pragma once

class noncopyable {
public:
    // 继承 noncopyable 的类无法使用复制构造函数和赋值构造函数
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    // 将构造函数设为 protected，无法实例化
    noncopyable() = default;
    ~noncopyable() = default;
};