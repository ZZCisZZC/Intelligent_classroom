#include "hardware.h"
#include <fstream>
#include <iostream>
#include <QString>

 float getTemperature(){


 }
 float getMoisture(){

 }
 float getIllumination(){

 }

 bool getHumidity(){

 }

float controlLight(int lightNum, float data) {
    if (lightNum < 1 || lightNum > 4) {
        return -1;  // 无效的 LED 编号
    }
    
    std::string path = "/sys/devices/platform/x210-led/led" + std::to_string(lightNum);
    std::ofstream ledFile(path);
    
    if (!ledFile.is_open()) {
        return -1;  // 无法打开设备文件
    }
    
    // 将状态转换为整数（0或1）
    int state = (data > 0.5) ? 1 : 0;
    ledFile << state;
    
    if (ledFile.fail()) {
        return -1;  // 写入失败
    }
    
    ledFile.close();
    return data;  // 返回设置的状态
}

 int controlMultiMedia(int mode){

 }

QString controlAirConditioner(int mode,int set){

}
