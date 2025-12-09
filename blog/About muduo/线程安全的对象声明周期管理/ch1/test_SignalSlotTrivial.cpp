#include "SignalSlotTrivial.h"
#include <iostream>

// g++ test_SignalSlotTrivial.cpp -o test.exe -std=c++11
void testSignalSlot() {
    // 创建一个信号，签名是 void(int)
    SignalTrivial<void(int)> signal;

    // 连接两个槽：使用 lambda 表达式
    signal.connect([](int x) {
        std::cout << "Slot 1: Received " << x << std::endl;
    });

    signal.connect([](int x) {
        std::cout << "Slot 2: Doubled value " << x * 2 << std::endl;
    });

    // 调用信号，传递参数 5
    std::cout << "Calling signal with argument 5:" << std::endl;
    signal.call(5);
}

int main() {
    testSignalSlot();
    return 0;
}
