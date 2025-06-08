#include "hardware.h"
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <ctime>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <termios.h>

// 定义ADC设备相关的宏
#define ADC_INPUT_PIN _IOW('S', 0x0c, unsigned long)

#define CH1            1
#define MASK12         0x0fff      /* 12-bit 数据掩码  */
#define BAD_THRESHOLD  3000        /* >3000 认为被干扰 */
#define MAX_ATTEMPT    10          /* 最多重采 10 次    */

// 打开 SHT31 设备并返回文件描述符
int opensht31() {
    const char* i2c_device = "/dev/i2c-1";  // I2C 设备文件
    const int sht31_addr = 0x44;           // SHT31 的 I2C 地址
    
    // 打开 I2C 设备
    int fd = open(i2c_device, O_RDWR);
    if (fd < 0) {
        return -1;  // 打开设备失败
    }
    
    // 设置从设备地址
    if (ioctl(fd, I2C_SLAVE, sht31_addr) < 0) {
        close(fd);
        return -1;  // 设置从设备地址失败
    }
    
    return fd;
}

float getTemperature() {
    int fd = opensht31();
    if (fd < 0) {
        return -1;
    }
    
    // 发送测量命令 (0x2C06 表示高重复性测量)
    unsigned char cmd[2] = {0x2C, 0x06};
    if (write(fd, cmd, 2) != 2) {
        close(fd);
        return -1;  // 发送命令失败
    }
    
    // 等待测量完成（SHT31 需要约 15ms）
    usleep(15000);
    
    // 读取 6 字节数据
    unsigned char data[6];
    if (read(fd, data, 6) != 6) {
        close(fd);
        return -1;  // 读取数据失败
    }
    
    // 关闭设备
    close(fd);
    
    // 计算温度值
    // 温度数据在 data[0] 和 data[1] 中
    unsigned int temp_raw = (data[0] << 8) | data[1];
    float temperature = -45.0f + 175.0f * temp_raw / 65535.0f;
    printf("Temperature:%f\n",temperature);
    return temperature;
}

float getHumidity() {
    int fd = opensht31();
    if (fd < 0) {
        return -1;
    }
    
    // 发送测量命令 (0x2C06 表示高重复性测量)
    unsigned char cmd[2] = {0x2C, 0x06};
    if (write(fd, cmd, 2) != 2) {
        close(fd);
        return -1;  // 发送命令失败
    }
    
    // 等待测量完成（SHT31 需要约 15ms）
    usleep(15000);
    
    // 读取 6 字节数据
    unsigned char data[6];
    if (read(fd, data, 6) != 6) {
        close(fd);
        return -1;  // 读取数据失败
    }
    
    // 关闭设备
    close(fd);
    
    // 计算湿度值
    // 湿度数据在 data[3] 和 data[4] 中
    unsigned int humidity_raw = (data[3] << 8) | data[4];
    float humidity = 100.0f * humidity_raw / 65535.0f;
    printf("Humidity:%f\n",humidity);
    return humidity;
}

float getIllumination() {
    int fd = open("/dev/adc", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open /dev/adc failed: %s\n", strerror(errno));
        return -1.0f;
    }

    unsigned int raw = 0;
    int attempt;

    for (attempt = 0; attempt < MAX_ATTEMPT; ++attempt) {

        /* 1) 切换到通道 1 */
        if (ioctl(fd, ADC_INPUT_PIN, CH1) < 0) {
            perror("ioctl");
            close(fd);
            return -1.0f;
        }

        /* 2) 读取 4 字节原始样本 */
        if (read(fd, &raw, sizeof(raw)) != sizeof(raw)) {
            perror("read");
            close(fd);
            return -1.0f;
        }

        raw &= MASK12;             /* 3) 取有效 12 位 */

        /* 4) 正常值就跳出循环 */
        if (raw < BAD_THRESHOLD)
            break;
    }

    close(fd);

    if (attempt == MAX_ATTEMPT)    /* 连续 10 次都异常 */
        return -1.0f;

    printf("raw:%d\n",raw);
    printf("ans:%f\n",(float)raw / 4095.0f);
        getSensor();
    /* 5) 线性归一化到 0~1（0-3000 → 0-1） */
    return (float)raw / 4095.0f;
}

bool getPerson(){

}

int controlLight(int lightNum, bool data) {
    if (lightNum < 0 || lightNum > 3) {
        return -1;  // 无效的 LED 编号
    }
    std::string path = "/sys/devices/platform/x210-led/led" + std::to_string(lightNum+1);
    std::ofstream ledFile(path);
    if (!ledFile.is_open()) {
        return -1;  // 无法打开设备文件
    }
    // 将状态转换为整数（0或1）
    int state = data ? 1 : 0;
    ledFile << state;
    if (ledFile.fail()) {
        return -1;  // 写入失败
    }
    ledFile.close();
    return state;  // 返回设置的状态
}

std::string getSensor() {
    // 打开串口设备
    int fd = open("/dev/s3c2410_serial1", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        std::string error = "{\"error\": \"Failed to open serial port\"}";
        std::cout << "Error: " << error << std::endl;
        return error;
    }

    // 配置串口参数
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(fd, &tty) != 0) {
        close(fd);
        std::string error = "{\"error\": \"Failed to get serial attributes\"}";
        std::cout << "Error: " << error << std::endl;
        return error;
    }

    // 设置波特率
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    // 设置其他参数
    tty.c_cflag |= (CLOCAL | CREAD);    // 忽略modem控制
    tty.c_cflag &= ~PARENB;             // 无校验位
    tty.c_cflag &= ~CSTOPB;             // 1个停止位
    tty.c_cflag &= ~CSIZE;              // 清除数据位掩码
    tty.c_cflag |= CS8;                 // 8位数据位
    tty.c_cflag &= ~CRTSCTS;            // 无硬件流控

    // 设置非阻塞读取超时
    tty.c_cc[VTIME] = 10;               // 1秒超时
    tty.c_cc[VMIN] = 0;                 // 不要求最小字符数

    // 设置为原始模式
    cfmakeraw(&tty);

    // 应用设置
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        close(fd);
        std::string error = "{\"error\": \"Failed to set serial attributes\"}";
        std::cout << "Error: " << error << std::endl;
        return error;
    }

    // 清空缓冲区
    tcflush(fd, TCIOFLUSH);

    // 发送命令
    const char* cmd = "GETSENSOR\r\n";
    if (write(fd, cmd, strlen(cmd)) != strlen(cmd)) {
        close(fd);
        std::string error = "{\"error\": \"Failed to send command\"}";
        std::cout << "Error: " << error << std::endl;
        return error;
    }

    // 等待数据准备
    usleep(100000);  // 等待100ms确保数据准备好

    // 循环读取直到接收完整的JSON数据
    std::string response;
    char buffer[128];
    int retries = 10;  // 最多尝试10次
    bool validJson = false;
    size_t jsonStart = std::string::npos;
    size_t jsonEnd = std::string::npos;

    while (retries-- > 0) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            response += buffer;
            
            // 查找JSON的开始和结束位置
            if (jsonStart == std::string::npos) {
                jsonStart = response.find("{");
            }
            if (jsonStart != std::string::npos) {
                jsonEnd = response.find("}", jsonStart);
                if (jsonEnd != std::string::npos) {
                    validJson = true;
                    break;
                }
            }
        }
        
        usleep(50000);  // 等待50ms后继续读取
    }
    
    // 关闭串口
    close(fd);
    printf("close fd\n");

    if (!validJson) {
        std::string error = "{\"error\": \"Failed to receive complete data\"}";
        std::cout << "Error: " << error << std::endl;
        return error;
    }

    // 提取有效的JSON数据
    std::string cleanResponse = response.substr(jsonStart, jsonEnd - jsonStart + 1);
    
    // 打印接收到的字符串
    std::cout << "Received from serial port: " << cleanResponse << std::endl;
    
    return cleanResponse;
}

