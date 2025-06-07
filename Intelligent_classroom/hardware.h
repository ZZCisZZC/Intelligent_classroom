#ifndef HARDWARE_H
#define HARDWARE_H

#include <QString>

float getTemperature();
float getHumidity();
float getIllumination();
bool getPerson();

int controlLight(int lightNum,bool data);
int controlMultiMedia(int mode);
QString controlAirConditioner(int mode,int set);

#endif // HARDWARE_H
