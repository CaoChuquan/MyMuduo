#pragma once

#include <string>
#include <time.h>

class Timestamp {
private:
    int64_t microSecondsSinceEpoch_ = 0;
public:
    Timestamp() {}
    explicit Timestamp(int64_t m) : microSecondsSinceEpoch_(m) {}
    ~Timestamp() {}
    // 静态方法，返回当前时间戳的 Timestamp 类
    static Timestamp now();
    // 返回当前时间
    std::string toString() const;
};