#include "mqttcloud.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sys/select.h>

int updateToCloud(std::string jsonToCloud){
    // 将jsonToCloud发送到云端,发送成功返回1，发送失败返回0
        // 首先检查设备文件是否存在
    if (access("/dev/s3c2410_serial0", F_OK) == -1) {
        printf("Device not found\n");
        return 0;
    }

    // 使用非阻塞方式打开串口设备
    int uart_fd = open("/dev/s3c2410_serial0", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (uart_fd < 0) {
        printf("Failed to open device\n");
        return 0;
    }

    // 配置串口参数
    struct termios uart_config;
    memset(&uart_config, 0, sizeof(uart_config));
    
    // 获取当前串口配置
    if (tcgetattr(uart_fd, &uart_config) != 0) {
        close(uart_fd);
        printf("getattr failed\n");
        return 0;
    }

    // 设置波特率
    cfsetispeed(&uart_config, B115200);
    cfsetospeed(&uart_config, B115200);

    // 设置其他串口参数
    uart_config.c_cflag |= (CLOCAL | CREAD);    // 启用接收器并忽略调制解调器控制线
    uart_config.c_cflag &= ~PARENB;             // 无奇偶校验
    uart_config.c_cflag &= ~CSTOPB;             // 1个停止位
    uart_config.c_cflag &= ~CSIZE;              // 清除数据位掩码
    uart_config.c_cflag |= CS8;                 // 8位数据位
    
    // 应用新的串口配置
    if (tcsetattr(uart_fd, TCSANOW, &uart_config) != 0) {
        close(uart_fd);
        printf("setattr failed\n");
        return 0;
    }

    // 准备发送的数据
    std::string message = "UPDATE\n" + jsonToCloud;
    
    // 发送数据
    ssize_t bytes_written = write(uart_fd, message.c_str(), message.length());
    if (bytes_written != static_cast<ssize_t>(message.length())) {
        close(uart_fd);
        printf("write failed\n");
        return 0;
    }

    // 等待并读取响应
    char response[64];
    memset(response, 0, sizeof(response));
    
    // 设置超时
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(uart_fd, &readfds);
    
    // 设置3秒超时
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    
    int select_result = select(uart_fd + 1, &readfds, NULL, NULL, &timeout);
    if (select_result == -1) {
        // select错误
        close(uart_fd);
        return -1;
    } else if (select_result == 0) {
        // 超时
        close(uart_fd);
        return -1;
    }
    
    // 有数据可读
    ssize_t bytes_read = read(uart_fd, response, sizeof(response) - 1);
    
    // 关闭串口
    close(uart_fd);

    // 检查响应是否为"SENDOK"
    if (bytes_read > 0 && strcmp(response, "SENDOK") == 0) {
        return 1;  // 发送成功
    }

    return 0;  // 发送失败
}

std::string getControlJson(){
    // 接收云端发来的控制指令并返回字符串
}