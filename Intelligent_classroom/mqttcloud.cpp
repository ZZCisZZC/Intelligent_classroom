#include "mqttcloud.h"
#include "controller.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sys/select.h>
#include <pthread.h>
#include <cstdlib>
#include <stdio.h>
#include <queue>

// 全局发送队列和简单的互斥标志
std::queue<std::string> sendQueue;
bool queueMutex = false;

int updateToCloud(std::string jsonToCloud){
    // 现在只是将数据添加到发送队列，由监听线程统一处理
    // printf("[DEBUG] updateToCloud: 添加数据到发送队列\n");
    addToSendQueue("UPDATE\n" + jsonToCloud);
    return 1;  // 暂时返回成功，实际发送结果需要其他方式获取
}

void addToSendQueue(const std::string& data) {
    // 简单的队列操作（在真实项目中应该使用真正的互斥锁）
    while (queueMutex) {
        usleep(1000); // 等待1ms
    }
    queueMutex = true;
    sendQueue.push(data);
    queueMutex = false;
    // printf("[DEBUG] 数据已添加到发送队列\n");
}

void startSerialListener(Controller* controller) {
    // 启动串口监听线程
    // printf("[DEBUG] 正在启动串口监听线程\n");
    pthread_t listenerThread;
    int result = pthread_create(&listenerThread, NULL, serialListenerThread, controller);
    if (result != 0) {
        // printf("[DEBUG] 创建线程失败，错误码: %d\n", result);
        return;
    }
    pthread_detach(listenerThread);  // 分离线程，让它在后台运行
    // printf("[DEBUG] 串口监听线程创建成功\n");
}

void* serialListenerThread(void* arg) {
    Controller* controller = (Controller*)(arg);
    // printf("[DEBUG] 串口监听线程已启动\n");
    
    int uart_fd = -1;  // 保持串口打开状态
    
    // 串口监听线程函数
    while (true) {
        // 检查串口是否已打开，如果没有则打开
        if (uart_fd < 0) {
            // 检查设备文件是否存在
            if (access("/dev/s3c2410_serial0", F_OK) == -1) {
                // printf("[DEBUG] 串口设备不存在，等待1秒后重试\n");
                sleep(1);
                continue;
            }

            // printf("[DEBUG] 正在打开串口设备\n");
            // 打开串口设备
            uart_fd = open("/dev/s3c2410_serial0", O_RDWR | O_NOCTTY | O_NONBLOCK);
            if (uart_fd < 0) {
                // printf("[DEBUG] 打开串口失败，等待1秒后重试\n");
                sleep(1);
                continue;
            }
            
            // printf("[DEBUG] 串口打开成功，正在配置参数\n");
            // 配置串口参数
            struct termios uart_config;
            memset(&uart_config, 0, sizeof(uart_config));
            
            if (tcgetattr(uart_fd, &uart_config) != 0) {
                // printf("[DEBUG] 获取串口配置失败\n");
                close(uart_fd);
                uart_fd = -1;
                sleep(1);
                continue;
            }

            // 设置波特率和参数
            cfsetispeed(&uart_config, B115200);
            cfsetospeed(&uart_config, B115200);
            uart_config.c_cflag |= (CLOCAL | CREAD);
            uart_config.c_cflag &= ~PARENB;
            uart_config.c_cflag &= ~CSTOPB;
            uart_config.c_cflag &= ~CSIZE;
            uart_config.c_cflag |= CS8;
            
            if (tcsetattr(uart_fd, TCSANOW, &uart_config) != 0) {
                // printf("[DEBUG] 设置串口配置失败\n");
                close(uart_fd);
                uart_fd = -1;
                sleep(1);
                continue;
            }

            // printf("[DEBUG] 串口配置成功，开始监听数据\n");
        }

        // 处理发送队列
        while (!queueMutex && !sendQueue.empty()) {
            queueMutex = true;
            std::string dataToSend = sendQueue.front();
            sendQueue.pop();
            queueMutex = false;
            
            // printf("[DEBUG] 准备发送数据\n");
            ssize_t bytes_written = write(uart_fd, dataToSend.c_str(), dataToSend.length());
            // if (bytes_written > 0) {
                // printf("[DEBUG] 成功发送 %d 字节\n", (int)bytes_written);
            // } else {
                // printf("[DEBUG] 发送失败\n");
            // }
        }

        // 监听接收数据
        std::string receivedData;
        
        fd_set readfds;
        struct timeval timeout;
        
        FD_ZERO(&readfds);
        FD_SET(uart_fd, &readfds);
        
        // 设置100ms超时
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        
        int select_result = select(uart_fd + 1, &readfds, NULL, NULL, &timeout);
        if (select_result == -1) {
            // printf("[DEBUG] select错误，重新打开串口\n");
            close(uart_fd);
            uart_fd = -1;
            continue;
        } else if (select_result == 0) {
            // 超时，继续循环
            continue;
        }
        
        // printf("[DEBUG] 检测到有数据可读\n");
        // 有数据可读
        char buffer[1024];
        ssize_t bytes_read = read(uart_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            // printf("[DEBUG] 接收到 %d 字节数据: %s\n", (int)bytes_read, buffer);
            receivedData += std::string(buffer);
            
            // 检查是否收到完整的命令（以换行符结束）
            size_t newlinePos = receivedData.find('\n');
            if (newlinePos != std::string::npos) {
                std::string command = receivedData.substr(0, newlinePos);
                receivedData = receivedData.substr(newlinePos + 1);
                
                // printf("[DEBUG] 收到完整命令:\n");
                
                // 检查是否是SET命令
                if (command.find("SET") == 0) {
                    // printf("[DEBUG] 识别到SET命令\n");
                    // 提取SET后面的JSON数据
                    std::string jsonStr = command.substr(3);
                    
                    // printf("[DEBUG] 提取到JSON数据\n");
                    
                    // 去除前后空格
                    jsonStr.erase(0, jsonStr.find_first_not_of(" \t\r\n"));
                    jsonStr.erase(jsonStr.find_last_not_of(" \t\r\n") + 1);
                    
                    // 验证JSON格式
                    if (isValidJson(jsonStr)) {
                        // printf("[DEBUG] JSON格式验证通过，准备调用Controller\n");
                        // 调用Controller的setControl函数
                        if (controller != nullptr) {
                            // printf("[DEBUG] 调用controller->setControl()\n");
                            controller->setControl(jsonStr);
                            // printf("[DEBUG] controller->setControl()调用完成\n");
                        } else {
                            // printf("[DEBUG] 错误：controller为空指针\n");
                        }
                    } else {
                        // printf("[DEBUG] JSON格式验证失败\n");
                    }
                } else {
                    // printf("[DEBUG] 不是SET命令，忽略\n");
                }
            }
        }
    }
    return NULL;
}

bool isValidJson(const std::string& jsonStr) {

    // printf("[DEBUG] 开始验证JSON: %s\n", jsonStr.c_str());
    
    if (jsonStr.empty()) {
        // printf("[DEBUG] JSON为空字符串\n");
        return false;
    }
    
    // 检查是否以{开始和}结束
    if (jsonStr[0] != '{' || jsonStr[jsonStr.length()-1] != '}') {
        // printf("[DEBUG] JSON格式错误：不是以{}包围\n");
        return false;
    }
    
    // 简单检查是否包含必要的字段
    if (jsonStr.find("\"state\"") == std::string::npos) {
        // printf("[DEBUG] JSON缺少'state'字段\n");
        return false;
    }
    if (jsonStr.find("\"led\"") == std::string::npos) {
        // printf("[DEBUG] JSON缺少'led'字段\n");
        return false;
    }
    if (jsonStr.find("\"air_conditioner\"") == std::string::npos) {
        // printf("[DEBUG] JSON缺少'air_conditioner'字段\n");
        return false;
    }
    if (jsonStr.find("\"multimedia\"") == std::string::npos) {
        // printf("[DEBUG] JSON缺少'multimedia'字段\n");
        return false;
    }
    
    // printf("[DEBUG] JSON验证通过\n");
    return true;  // 简化验证，只检查基本结构
}