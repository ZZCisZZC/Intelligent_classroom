#include "controller.h"
#include "sensor.h"
#include "hardware.h"
#include "mqttcloud.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QByteArray>

Controller::Controller(QObject* parent) : QObject(parent) {
    timer = new QTimer(this);
    m_upload = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Controller::getSensorData); // 每十秒触发一次
    connect(timer, &QTimer::timeout, this, &Controller::generalControl);
    connect(m_upload, &QTimer::timeout, this, &Controller::uploadData);
    timer->start(10000);        // 每10秒读取一次
    m_upload->start(6000);

    m_sleepTimer = new QTimer(this);
    m_sleepTimer->setSingleShot(true);

    m_offTimer = new QTimer(this);
    m_offTimer->setSingleShot(true);

    connect(m_sleepTimer, &QTimer::timeout, this, [=]() {
        if (Sensor::instance()->multimediamode() == 1)
            controlMultiMedia(2); // 1 -> 2
    });
    connect(m_offTimer, &QTimer::timeout, this, [=]() {
        if (Sensor::instance()->multimediamode() == 2)
            controlMultiMedia(0); // 2 -> 0
    });
}

void Controller::generalControl() {
    bool n_automode = Sensor::instance()->automode();
    bool n_person = Sensor::instance()->person();
    // 自动模式无人需要做更改
    if ( !n_person && n_automode) {
        for (int i = 0; i < 4; ++i){
            int result = getLightState(i, false);
            if ( result == -1){
                qDebug() << "操作失败！";
            }
            else{
                Sensor::instance()->updatalightstate(false, i); // 关灯
            }
        }
        Sensor::instance()->updateairconditioner(false, Sensor::instance()->airconditionermode(),
                                                 Sensor::instance()->airconditionerset());
    }
}

void Controller::getSensorData() {
    std::string jsonStr = getSensor();
    QString qJsonStr = QString::fromStdString(jsonStr);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(qJsonStr.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << error.errorString();
        return;
    }

    if (!doc.isObject()) {
        qWarning() << "JSON 不是对象格式";
        return;
    }

    QJsonObject obj = doc.object();

    float temp = obj.value("temp").toDouble();
    float humidity = obj.value("humidity").toDouble();
    float lux = QString::number(obj.value("lux").toDouble(), 'f', 2).toFloat();
    bool person = obj.value("person").toInt() == 1;

    Sensor::instance()->update(temp, humidity, lux, person);
}

int Controller::getLightState(int lightNum,bool data) {
    return controlLight(lightNum, data);
}

void Controller::controlAirConditioner(bool state, int mode, int set) {
    qDebug() << "控制空调: " << state << " 模式: " << mode << " 挡位: " << set;

    Sensor::instance()->updateairconditioner(state, mode, set);
}

void Controller::controlMultiMedia(int mode) {
    Sensor::instance()->updatemultimediamode(mode);
    qDebug() << "设置多媒体模式: " << mode;

    if (mode == 1) {
        m_offTimer->stop();
        m_sleepTimer->stop();
        m_sleepTimer->start(5000);

    } else if (mode == 2) {
        m_sleepTimer->stop();
        m_offTimer->stop();
        m_offTimer->start(5000);

    } else if (mode == 0) {
        // 彻底关闭，所有定时器清除
        m_sleepTimer->stop();
        m_offTimer->stop();
    }
}

void Controller::uploadData() {
    QJsonObject timeObj;
    timeObj["year"] = Sensor::instance()->getyear();
    timeObj["month"] = Sensor::instance()->getmonth();
    timeObj["day"] = Sensor::instance()->getday();
    timeObj["hour"] = Sensor::instance()->gethour();
    timeObj["minute"] = Sensor::instance()->getminute();

    QJsonObject sensorObj;
    sensorObj["temp"] = Sensor::instance()->temperature();
    sensorObj["humidity"] = Sensor::instance()->moisture();
    sensorObj["lux"] = Sensor::instance()->illumination();
    sensorObj["person"] = (Sensor::instance()->person()) ? "true" : "false";

    QJsonObject ledObj;
    ledObj["led1"] = (Sensor::instance()->lightstate(0)) ? 1 : 0;
    ledObj["led2"] = (Sensor::instance()->lightstate(1)) ? 1 : 0;
    ledObj["led3"] = (Sensor::instance()->lightstate(2)) ? 1 : 0;
    ledObj["led4"] = (Sensor::instance()->lightstate(3)) ? 1 : 0;

    QJsonObject airObj;
    airObj["state"] = (Sensor::instance()->airconditionerstate()) ? "on" : "off";
    airObj["mode"] = (Sensor::instance()->airconditionermode()) ? "heat" : "cool";
    airObj["level"] = Sensor::instance()->airconditionerset();

    QString mmodeStr;
    switch (Sensor::instance()->multimediamode()) {
        case 0: mmodeStr = "off"; break;
        case 1: mmodeStr = "on"; break;
        case 2: mmodeStr = "standby"; break;
    }

    QJsonObject stateObj;
    stateObj["led"] = ledObj;
    stateObj["air_conditioner"] = airObj;
    stateObj["multimedia"] = mmodeStr;

    QJsonObject root;
    root["time"] = timeObj;
    root["device_id"] = "classroom-node-01";
    root["sensor_data"] = sensorObj;
    root["state"] = stateObj;

    QJsonDocument doc(root);
    QString jsonStr = doc.toJson(QJsonDocument::Indented);
    qDebug() << jsonStr;
    QByteArray byteArray = doc.toJson(QJsonDocument::Indented);

    // updateToCloud(byteArray.constData())
}


