#ifndef HARDWARE_H
#define HARDWARE_H

#include <QString>

float getTemperature();
float getHumidity();
float getIllumination();
bool getPerson();

float controlLight(int lightNum,float data);
int controlMultiMedia(int mode);
QString controlAirConditioner(int mode,int set);

#endif // HARDWARE_H
