#include "hardware.h"
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

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

float getIllumination(){

}

bool getPerson(){

}

int controlLight(int lightNum, bool data) {
    if (lightNum < 0 || lightNum > 3) {
        return -1;  // 无效的 LED 编号
    }
    
    std::string path = "/sys/devices/platform/x210-led/led" + std::to_string(lightNum);
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

 int controlMultiMedia(int mode){

 }

QString controlAirConditioner(int mode,int set){

}
