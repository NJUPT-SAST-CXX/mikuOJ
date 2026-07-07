#include <iostream>

int main() {
    long long a, b;
    std::cin >> a >> b;
    std::cout << a * b << std::endl;  // 错误：应为 a + b
    return 0;
}
