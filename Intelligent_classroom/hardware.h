#ifndef HARDWARE_H
#define HARDWARE_H

#include <QString>
#include <string>

float getTemperature();//获取温度
float getHumidity();//获取湿度
float getIllumination();//获取光照
bool getPerson();//获取人数

int controlLight(int lightNum,bool data);//控制灯光
<<<<<<< Updated upstream
std::string getSensor();//获取传感器数据
=======
>>>>>>> Stashed changes

#endif // HARDWARE_H
