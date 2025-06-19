#include "controller.h"
#include "sensor.h"
#include "hardware.h"
#include "timefile.h"
#include "mqttcloud.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QByteArray>

Controller::Controller(QObject* parent) : QObject(parent) {
    timer = new QTimer(this);
    m_upload = new QTimer(this);
    m_clock = new QTimer(this);
    m_timetofile = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Controller::getSensorData); 
    connect(timer, &QTimer::timeout, this, &Controller::generalControl);
    connect(m_upload, &QTimer::timeout, this, &Controller::uploadData);
    connect(m_clock, &QTimer::timeout, this, &Controller::timeHandler);
    connect(m_timetofile, &QTimer::timeout, this, &Controller::uploadTimeToHardware);
    timer->start(1000);        // 每1秒读取一次
    m_upload->start(1000);
    m_clock->start(1000);
    m_timetofile->start(1000);

    m_sleepTimer = new QTimer(this);
    m_sleepTimer->setSingleShot(true);

    m_offTimer = new QTimer(this);
    m_offTimer->setSingleShot(true);

    m_airConditionerOffTimer = new QTimer(this);
    m_airConditionerOffTimer->setSingleShot(true);

    // 连接信号槽，用于跨线程启动定时器
    connect(this, &Controller::startSleepTimer, this, &Controller::onStartSleepTimer);
    connect(this, &Controller::startOffTimer, this, &Controller::onStartOffTimer);
    connect(this, &Controller::startAirConditionerOffTimer, this, &Controller::onStartAirConditionerOffTimer);

    connect(m_sleepTimer, &QTimer::timeout, this, [=]() {
        // printf("*** 睡眠定时器触发 ***\n");
        // printf("当前多媒体模式=%d (期望为1)\n", Sensor::instance()->multimediamode());
        if (Sensor::instance()->multimediamode() == 1) {
            // printf("执行: 开启->睡眠 转换\n");
            controlMultiMedia(2); // 1 -> 2
        } else {
            // printf("跳过转换: 多媒体模式已改变\n");
        }
    });

    connect(m_offTimer, &QTimer::timeout, this, [=]() {
        // printf("*** 关闭定时器触发 ***\n");
        // printf("当前多媒体模式=%d (期望为2)\n", Sensor::instance()->multimediamode());
        if (Sensor::instance()->multimediamode() == 2) {
            // printf("执行: 睡眠->关闭 转换\n");
            controlMultiMedia(0); // 2 -> 0
        } else {
            // printf("跳过转换: 多媒体模式已改变\n");
        }
    });
    connect(m_airConditionerOffTimer, &QTimer::timeout, this, [=]() {
        // printf("*** 空调关闭定时器触发 ***\n");
        // printf("当前空调状态=%s (期望为开启)\n", Sensor::instance()->airconditionerstate() ? "开启" : "关闭");
        if (Sensor::instance()->airconditionerstate()) {
            // printf("执行: 空调自动关闭\n");
            controlAirConditioner(false, Sensor::instance()->airconditionermode(), Sensor::instance()->airconditionerset());
        } else {
            // printf("跳过关闭: 空调状态已改变\n");
        }
    });

    getTimeFromHardware();
}

void Controller::generalControl() {
    bool n_automode = Sensor::instance()->automode();
    bool n_person = Sensor::instance()->person();
    int n_multimedia = Sensor::instance()->multimediamode();
    bool n_airconditioner = Sensor::instance()->airconditionerstate();
    
    // printf("generalControl: 自动模式=%s 有人=%s 多媒体状态=%d 空调状态=%s\n", 
    //        n_automode ? "true" : "false", 
    //        n_person ? "true" : "false", 
    //        n_multimedia,
    //        n_airconditioner ? "开启" : "关闭");
    
    if (!n_person && n_automode) {
        for (int i = 0; i < 4; ++i){
            int result = getLightState(i, false);
            if (result == -1){
                // printf("操作失败！\n");
            } else {
                Sensor::instance()->updatalightstate(false, i); // 关灯
            }
        }
        if (n_airconditioner) {
            if (!m_airConditionerOffTimer->isActive()) {
                // printf("generalControl: 无人检测，空调开启状态，启动3秒关闭定时器\n");
                emit startAirConditionerOffTimer();
            } else {
                // printf("generalControl: 无人检测，空调开启状态，但关闭定时器已在运行\n");
            }
        } else {
            // printf("generalControl: 无人检测，空调关闭状态，不启动定时器\n");
        }
        if (n_multimedia == 1) {
            if (!m_sleepTimer->isActive()) {
                // printf("generalControl: 无人检测，多媒体开启状态，启动5秒睡眠定时器\n");
                emit startSleepTimer();
            } else {
                // printf("generalControl: 无人检测，多媒体开启状态，但睡眠定时器已在运行\n");
            }
        } else if (n_multimedia == 2) {
            if (!m_offTimer->isActive()) {
                // printf("generalControl: 无人检测，多媒体睡眠状态，启动5秒关闭定时器\n");
                emit startOffTimer();
            } else {
                // printf("generalControl: 无人检测，多媒体睡眠状态，但关闭定时器已在运行\n");
            }
        } else {
            // printf("generalControl: 无人检测，多媒体关闭状态，不启动定时器\n");
        }
    } else if (n_person && n_automode) {
        if (m_sleepTimer->isActive() || m_offTimer->isActive() || m_airConditionerOffTimer->isActive()) {
            // printf("有人进入：停止所有自动切换定时器，保持当前状态\n");
            m_sleepTimer->stop();
            m_offTimer->stop();
            m_airConditionerOffTimer->stop();
        }
    }
}

void Controller::getSensorData() {
    std::string jsonStr = getSensor();
    QString qJsonStr = QString::fromStdString(jsonStr);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(qJsonStr.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        // printf("JSON 解析错误: %s\n", error.errorString().toLocal8Bit().data());
        return;
    }

    if (!doc.isObject()) {
        // printf("JSON 不是对象格式\n");
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
    // printf("=== controlAirConditioner调用 ===\n");
    // printf("参数: 状态=%s 模式=%d 挡位=%d\n", state ? "开" : "关", mode, set);
    // printf("当前状态: 自动模式=%s 有人=%s\n", Sensor::instance()->automode() ? "true" : "false", Sensor::instance()->person() ? "true" : "false");

    Sensor::instance()->updateairconditioner(state, mode, set);
    // printf("空调状态已更新为: %s\n", state ? "开启" : "关闭");

    bool timerWasActive = m_airConditionerOffTimer->isActive();
    m_airConditionerOffTimer->stop();
    // printf("空调定时器状态: 之前%s 现在停止\n", timerWasActive ? "运行" : "停止");

    if (!Sensor::instance()->automode()) {
        // printf("手动控制模式，不启动自动关闭定时器\n");
        // printf("=== controlAirConditioner结束 ===\n");
        return;
    }

    // printf("自动控制模式，继续检查逻辑\n");

    if (Sensor::instance()->person()) {
        // printf("有人在场，维持当前状态，不启动定时器\n");
        // printf("=== controlAirConditioner结束 ===\n");
        return;
    }

    // printf("无人在场，开始自动控制逻辑，当前状态=%s\n", state ? "开启" : "关闭");
    if (state) {
        // printf("空调开启状态，发送启动关闭定时器信号\n");
        emit startAirConditionerOffTimer();
        // printf("空调关闭定时器信号已发送\n");
    } else {
        // printf("空调关闭状态，不启动任何定时器\n");
    }
    // printf("=== controlAirConditioner结束 ===\n");
}

void Controller::controlMultiMedia(int mode) {
    // printf("=== controlMultiMedia调用 ===\n");
    // printf("参数: 模式=%d\n", mode);
    // printf("当前状态: 自动模式=%s 有人=%s\n", 
    //        Sensor::instance()->automode() ? "true" : "false",
    //        Sensor::instance()->person() ? "true" : "false");
    
    // 更新多媒体状态
    Sensor::instance()->updatemultimediamode(mode);
    // printf("多媒体状态已更新为: %d\n", mode);

    // 无论什么情况，先停止所有定时器
    bool sleepWasActive = m_sleepTimer->isActive();
    bool offWasActive = m_offTimer->isActive();
    m_sleepTimer->stop();
    m_offTimer->stop();
    // printf("定时器状态: 睡眠定时器之前%s 关闭定时器之前%s 现在全部停止\n", 
    //        sleepWasActive ? "运行" : "停止",
    //        offWasActive ? "运行" : "停止");
    
    // 如果是手动控制模式，不启动任何定时器，直接返回
    if (!Sensor::instance()->automode()) {
        // printf("手动控制模式，不启动自动切换定时器\n");
        // printf("=== controlMultiMedia结束 ===\n");
        return;
    }
    
    // printf("自动控制模式，继续检查逻辑\n");
    
    // 如果有人，不启动定时器（维持手动控制逻辑）
    if (Sensor::instance()->person()) {
        // printf("有人在场，维持当前状态，不启动定时器\n");
        // printf("=== controlMultiMedia结束 ===\n");
        return;
    }
    
    // 无人时的自动控制逻辑
    // printf("无人在场，开始自动控制逻辑，当前模式=%d\n", mode);
    if (mode == 1) {
        // 开启状态 -> 5秒后进入睡眠
        // printf("多媒体开启状态，发送启动睡眠定时器信号\n");
        emit startSleepTimer(); // 使用信号代替直接启动
        // printf("睡眠定时器信号已发送\n");
    } else if (mode == 2) {
        // 睡眠状态 -> 5秒后关闭
        // printf("多媒体睡眠状态，发送启动关闭定时器信号\n");
        emit startOffTimer(); // 使用信号代替直接启动
        // printf("关闭定时器信号已发送\n");
    } else if (mode == 0) {
        // printf("多媒体关闭状态，不启动任何定时器\n");
    }
    // printf("=== controlMultiMedia结束 ===\n");
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
    //qDebug() << jsonStr;
    QByteArray byteArray = doc.toJson(QJsonDocument::Indented);

    updateToCloud(byteArray.constData());
}

void Controller::setControl(std::string jsonStr) {
    QString qJsonStr = QString::fromStdString(jsonStr);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(qJsonStr.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        // printf("JSON 解析错误: %s\n", error.errorString().toLocal8Bit().data());
        return;
    }

    if (!doc.isObject()) {
        // printf("JSON 不是对象格式\n");
        return;
    }
    QJsonObject obj = doc.object();

    QJsonObject state = obj["state"].toObject();
    
    // 处理LED控制 - 只更新状态，让现有的UI反应机制来控制硬件
    QJsonObject leds = state["led"].toObject();
    // printf("[DEBUG] 开始处理LED控制\n");
    for (int i = 0; i < 4; ++i) {
        QString key = QString("led%1").arg(i + 1);
        bool ledState = leds[key].toInt() == 1;
        // printf("[DEBUG] 设置LED %d 状态为: %s\n", i+1, ledState ? "开" : "关");
        
        // 只更新Sensor状态，不直接控制硬件
        Sensor::instance()->updatalightstate(ledState, i);
        
        int result = getLightState(i, ledState);
        if(result == -1) {
            // printf("[DEBUG] LED %d 控制失败\n", i+1);
        } else {
            // printf("[DEBUG] LED %d 控制成功\n", i+1);
        }
    }

    // 处理空调控制
    QJsonObject ac = state["air_conditioner"].toObject();
    bool acState = ac["state"].toString() == "on";
    QString modeStr = ac["mode"].toString();
    int mode = (modeStr == "cool") ? 0 : (modeStr == "heat" ? 1 : -1);
    int level = ac["level"].toInt();
    // printf("[DEBUG] 设置空调: 状态=%s, 模式=%s, 档位=%d\n", 
    //        acState ? "开" : "关", modeStr.toStdString().c_str(), level);
    
    // 使用现有的控制函数
    controlAirConditioner(acState, mode, level);

    // 处理multimedia字段 - setControl设置后继续按照自动/手动模式逻辑运行
    int multimediaMode = 0;
    QJsonValue multimediaValue = state["multimedia"];
    if (multimediaValue.isString()) {
        QString modeStr = multimediaValue.toString().toLower();
        if (modeStr == "off") {
            multimediaMode = 0;  // 关闭
        } else if (modeStr == "on") {
            multimediaMode = 1;  // 开启
        } else if (modeStr == "standby") {
            multimediaMode = 2;  // 睡眠
        }
    } else if (multimediaValue.isDouble()) {
        multimediaMode = multimediaValue.toInt();
    }
    // printf("[DEBUG] 远程控制设置多媒体模式: %d\n", multimediaMode);
    
    // 使用controlMultiMedia函数，它会根据当前的自动/手动模式执行相应逻辑
    controlMultiMedia(multimediaMode);
    
    // printf("[DEBUG] Controller::setControl 处理完成\n");
}

void Controller::getTimeFromHardware() {
    // 时间初始化
    std::string n_time = getTimeFromFile();
    QString qTime = QString::fromStdString(n_time);
    // printf("时间字符串: %s\n", qTime.toLocal8Bit().data());
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(qTime.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        // printf("JSON 解析错误: %s\n", error.errorString().toLocal8Bit().data());
        return;
    }

    if (!doc.isObject()) {
        // printf("JSON 不是对象格式\n");
        return;
    }
    QJsonObject obj = doc.object();

    int year = obj.value("year").toInt();
    int month = obj.value("month").toInt();
    int day = obj.value("day").toInt();
    int hour = obj.value("hour").toInt();
    int minute = obj.value("minute").toInt();

    Sensor::instance()->updatetime(year, month, day, hour, minute);
}

// 判断是否为闰年
bool Controller::isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 获取指定年月的天数
int Controller::getDaysInMonth(int year, int month) {
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return isLeapYear(year) ? 29 : 28;
        default:
            return 30;
    }
}

void Controller::timeHandler() {
    int n_year = Sensor::instance()->getyear();
    int n_month = Sensor::instance()->getmonth();
    int n_day = Sensor::instance()->getday();
    int n_hour = Sensor::instance()->gethour();
    int n_minute = Sensor::instance()->getminute();

    n_minute += 10; //每十分钟（1s）更新一次显示时间

    if (n_minute >= 60) {
        n_minute -= 60;
        n_hour += 1;
    }
    if (n_hour >= 24) {
        n_hour -= 24;
        n_day += 1;
    }
    
    // 获取当前月份的天数
    int daysInCurrentMonth = getDaysInMonth(n_year, n_month);
    if (n_day > daysInCurrentMonth) {
        n_day = 1;
        n_month += 1;
    }
    if (n_month > 12) {
        n_month = 1;
        n_year += 1;
    }

    Sensor::instance()->updatetime(n_year, n_month, n_day, n_hour, n_minute);
}
 void Controller::uploadTimeToHardware() {
     QJsonObject timeObj;
     timeObj["year"] = Sensor::instance()->getyear();
     timeObj["month"] = Sensor::instance()->getmonth();
     timeObj["day"] = Sensor::instance()->getday();
     timeObj["hour"] = Sensor::instance()->gethour();
     timeObj["minute"] = Sensor::instance()->getminute();

     QJsonDocument doc(timeObj);
     QString jsonStr = doc.toJson(QJsonDocument::Indented);
     QByteArray byteArray = doc.toJson(QJsonDocument::Indented);

    storeTimeToFile(byteArray.constData());
 }

void Controller::onStartSleepTimer() {
    // printf("*** 收到启动睡眠定时器信号 ***\n");
    m_sleepTimer->start(5000);
    // printf("睡眠定时器已在主线程启动，状态=%s\n", m_sleepTimer->isActive() ? "运行中" : "失败");
}

void Controller::onStartOffTimer() {
    // printf("*** 收到启动关闭定时器信号 ***\n");
    m_offTimer->start(5000);
    // printf("关闭定时器已在主线程启动，状态=%s\n", m_offTimer->isActive() ? "运行中" : "失败");
}

void Controller::onStartAirConditionerOffTimer() {
    // printf("*** 收到启动空调关闭定时器信号 ***\n");
    m_airConditionerOffTimer->start(5000);  // 5秒定时器
    // printf("空调关闭定时器已在主线程启动，状态=%s\n", m_airConditionerOffTimer->isActive() ? "运行中" : "失败");
}

// 调试阶段用的输出已注释