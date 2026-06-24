#pragma once

#include <unistd.h>
#include <sys/syscall.h>

/* 用于高效获取当前线程的 ID。它通过线程局部存储（TLS）
   和系统调用缓存来减少频繁调用 gettid() 系统调用的开销 */

namespace CurrentThread {
    extern __thread int t_cachedTid;

    void cacheTid();

    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }
}