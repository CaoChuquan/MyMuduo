#include <iostream>
#include <string>
#include "InetAddress.h"
#include "Logger.h"
#include "InetAddress.h"
#include "CurrentThread.h"

int main() {
    using namespace std;

    // 测试日志
    Logger::instance().setLogLevel(INFO);
    Logger::instance().log("Test log");
    
    InetAddress testAddr(8888);
    cout << testAddr.toIpPort() << endl;

    cout << "Current Tid: " << CurrentThread::tid();
    cout << endl;
    
    return 0;
}