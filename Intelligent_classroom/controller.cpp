#include "controller.h"
#include "sensor.h"
#include "hardware.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

Controller::Controller(QObject* parent) : QObject(parent) {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Controller::getSensorData); // 每十秒触发一次
    connect(timer, &QTimer::timeout, this, &Controller::generalControl);
    timer->start(10000);  // 每10秒读取一次
}

void Controller::generalControl() {
    bool n_automode = Sensor::instance()->automode();
    bool n_person = Sensor::instance()->person();
    // 自动模式无人需要做更改
    if ( !n_person && n_automode) {
        for (int i = 0; i < 4; ++i){
            Sensor::instance()->updatalightstate(false, i); // 关灯
            Sensor::instance()->updateairconditioner(false, Sensor::instance()->airconditionermode(),
                                                     Sensor::instance()->airconditionerset());
        }
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
    float lux = obj.value("lux").toDouble();
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
