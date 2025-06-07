#ifndef HARDWARE_H
#define HARDWARE_H

#include <QString>

float getTemperature();//获取温度
float getHumidity();//获取湿度
float getIllumination();//获取光照
bool getPerson();//获取人数

int controlLight(int lightNum,bool data);//控制灯光

#endif // HARDWARE_H
