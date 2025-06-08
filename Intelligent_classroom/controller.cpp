#include "controller.h"
#include "sensor.h"
#include "hardware.h"
#include <QDebug>

Controller::Controller(QObject* parent) : QObject(parent) {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Controller::getSensorData); // 每十秒触发一次
    timer->start(500);  // 每0.5秒读取一次
}

void Controller::getSensorData() {
    // 只是模拟一下，这里是预留接口的位置
    float n_temp = Sensor::instance()->temperature();
    float n_mois = Sensor::instance()->moisture();
    float n_illum = Sensor::instance()->illumination();
    bool n_persona = Sensor::instance()->person();

    // 实际情况这四个参数的更新要通过函数调取
    n_temp = getTemperature();
    n_mois = getHumidity();
    n_illum = getIllumination();
    n_persona = !n_persona;

    Sensor::instance()->update(n_temp, n_mois, n_illum, n_persona);
}

int Controller::getLightState(int lightNum,bool data) {
    return controlLight(lightNum, data);
}
