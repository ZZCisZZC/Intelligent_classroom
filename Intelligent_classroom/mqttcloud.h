#ifndef MQTTCLOUD_H
#define MQTTCLOUD_H

#include <string>
#include <queue>

class Controller;  // 前向声明

// 全局发送队列和锁
extern std::queue<std::string> sendQueue;
extern bool queueMutex;

int updateToCloud(std::string jsonToCloud);
void getControlJson();
void startSerialListener(Controller* controller);  // 启动串口监听线程
void* serialListenerThread(void* arg);             // 串口监听线程函数(使用pthread)
bool isValidJson(const std::string& jsonStr);      // JSON格式验证
void addToSendQueue(const std::string& data);      // 添加数据到发送队列

#endif // MQTTCLOUD_H
