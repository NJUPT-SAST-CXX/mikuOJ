// 尝试建立网络连接 —— seccomp 无 socket → SIGSYS → SV
#include <sys/socket.h>
#include <netinet/in.h>
int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    return s < 0 ? 1 : 0;
}
