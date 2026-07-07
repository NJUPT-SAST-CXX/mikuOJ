// fork 炸弹 —— Strict profile 无 clone/fork → SIGSYS → SV
#include <unistd.h>
int main() { for (;;) fork(); }
