#ifndef MQTTCLOUD_H
#define MQTTCLOUD_H

#include <string>

int updateToCloud(std::string jsonToCloud);

int setControl(std::string setByCloud);

#endif // MQTTCLOUD_H
