// 尝试 #include 宿主敏感文件 —— 编译沙箱内 /etc/passwd 不可见 → 编译失败 (CE)
#include "/etc/passwd"

int main() { return 0; }
