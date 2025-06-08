#ifndef MQTTCLOUD_H
#define MQTTCLOUD_H

#include <string>

int updateToCloud(std::string jsonToCloud);

std::string getControlJson();

#endif // MQTTCLOUD_H
