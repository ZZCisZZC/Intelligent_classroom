#ifndef SENSOR_H
#define SENSOR_H

#include <QString>
#include <QObject>

class Sensor: public QObject
{
    Q_OBJECT

public:
    // 外部访问函数
    static Sensor* instance();
    bool automode() const;
    float temperature() const;
    float moisture() const;
    float illumination() const;
    bool person() const;
    bool lightstate(int index) const;
    bool airconditionerstate() const;
    int airconditionermode() const;
    int airconditionerset() const;
    void updateautomode(bool state);
    void update(float temp, float mois, float illum, bool person);
    void updatalightstate(bool state, int index);
    void updateairconditioner(bool state, int mode, int set);

signals:
    // 信号
    void temperatureChanged(float);
    void moistureChanged(float);
    void illuminationChanged(float);
    void personChanged(bool);
    void lightStateChanged(int index, bool state);
    void airconditionerStateChanged(bool state);
    void airconditionerModeChanged(int mode);
    void airconditionerSetChanged(int set);

private:
    bool m_automode = false;            // 模式状态，true为自动，false为手动
    float m_temperature;                // 温度
    float m_moisture;                   // 预留
    float m_illumination;               // 亮度
    bool m_person;                      // true表示有人，false表示无人
    bool m_lightStates[4] = {false};    // 灯的状态
    bool m_airconditionerState = false; // 空调开关
    int m_airconditionerMode = 0;       // 空调模式，0为制冷，1为制热
    int m_airconditionerSet = 1;        // 空调挡位（1，2，3）
    explicit Sensor(QObject *parent = nullptr);
};

#endif // SENSOR_H
// 不能把头文件endif的备注删除！！！
