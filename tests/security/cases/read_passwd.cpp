// 尝试读取宿主 /etc/passwd —— 沙箱内不应存在（无泄漏）
#include <fstream>
#include <iostream>
int main() {
    std::ifstream f("/etc/passwd");
    std::string line;
    while (std::getline(f, line)) std::cout << line << "\n";
    return 0;
}
