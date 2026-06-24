// logger.cpp
#include "Logger.h"

using std::cout;
using std::endl;

// 获取唯一的日志实例
Logger& Logger::instance() {
    static Logger log;
    return log;
}

// 设置日志级别
void Logger::setLogLevel(int level) {
    logLevel_ = level;
}

// 写日志，[level][timestamp] msg
void Logger::log(std::string msg) {
    switch (logLevel_) {
    case INFO:
        cout << "[INFO] ";
        break;
    case ERROR:
        cout << "[ERROR] ";
        break;
    case FATAL:
        cout << "[FATAL] ";
        break;
    case DEBUG:
        cout << "[DEBUG] ";
        break;
    default:
        break;
    }
    cout << Timestamp::now().toString() << " : " << msg << endl;
}