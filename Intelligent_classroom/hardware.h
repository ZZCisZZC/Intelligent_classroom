#ifndef HARDWARE_H
#define HARDWARE_H

#include <QString>

float getTemperature();
float getHumidity();
float getIllumination();
bool getPerson();

int controlLight(int lightNum,int data);
int controlMultiMedia(int mode);
QString controlAirConditioner(int mode,int set);

#endif // HARDWARE_H
