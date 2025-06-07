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
    float temperature() const;
    float moisture() const;
    float illumination() const;
    bool person() const;
    bool lightstate(int index) const;
    void update(float temp, float mois, float illum, bool person);
    void updatalightstate(bool state, int index);

signals:
    // 信号
    void temperatureChanged(float);
    void moistureChanged(float);
    void illuminationChanged(float);
    void personChanged(bool);

private:
    float m_temperature;                // 温度
    float m_moisture;                   // 预留
    float m_illumination;               // 亮度
    bool m_person;                      // true表示有人，false表示无人
    bool m_lightStates[4] = {false};    // 灯的状态
    explicit Sensor(QObject *parent = nullptr);
};


#endif // SENSOR_H
// 不能把头文件endif的备注删除！！！
