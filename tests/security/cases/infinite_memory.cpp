// 内存炸弹 —— cgroup memory.max 触发 OOM → MLE
#include <vector>

int main() {
    std::vector<char*> blocks;
    for (;;) {
        char* p = new char[1024 * 1024];
        for (int i = 0; i < 1024 * 1024; i += 4096) p[i] = 1;
        blocks.push_back(p);
    }
}
