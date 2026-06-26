# MyMuduo

#### 介绍
从零重写 muduo 网络库，深入理解 Reactor 模型与 C++ 高性能网络编程

#### 项目背景
muduo 是陈硕老师开发的一款基于 C++ 的高性能网络库，采用非阻塞 I/O 和事件驱动设计，支持多线程和多 Reactor 模型，是学习现代 C++ 网络编程的绝佳教材。

本项目旨在从零重写 muduo 网络库的核心组件，目标包括：

深入理解阻塞/非阻塞、同步/异步、五种 I/O 模型及 epoll 原理

掌握 Reactor 反应器模型的核心设计思想

实践基于 epoll + 线程池的事件驱动编程

探索现代 C++（C++11/14/17）在高性能网络编程中的应用

#### 项目特点
- 核心架构：基于 Reactor 模式，实现非阻塞 I/O 与事件驱动

- 多线程模型：采用 One Loop Per Thread 模型，充分利用多核 CPU 性能

- 现代 C++ 重构：使用 C++11 特性替代 Boost 依赖（std::shared_ptr、std::function、std::bind 等）

- 清晰的层次设计：网络层与业务层分离，开发者只需关注业务逻辑

- 详尽的代码注释：对核心类和关键算法添加中文注释，方便学习

#### 关于重写
本次重写严格遵循 muduo 的核心设计思想，同时在实现上做了以下调整和改进：

1. 去除 Boost 依赖
原 muduo 大量使用 Boost 库（boost::shared_ptr、boost::function、boost::bind 等）。重写版本全面使用 C++11 标准库替代：

原 Boost 组件	替代方案
boost::shared_ptr	std::shared_ptr
boost::weak_ptr	std::weak_ptr
boost::function	std::function
boost::bind	std::bind + lambda
boost::noncopyable	= delete 声明
2. 内存管理优化
全面使用智能指针管理资源，替代手动的 new/delete，提高代码安全性和可维护性。

3. 回调机制
使用 std::function 和 lambda 表达式，代码更加简洁现代。

4. 定时器实现
使用 timerfd 实现定时器管理，与 EventLoop 的 epoll 统一集成，更加高效。