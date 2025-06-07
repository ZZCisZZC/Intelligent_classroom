#ifndef SENSOR_H
#define SENSOR_H

#include <QString>
#include <QObject>

class Sensor: public QObject
{
    Q_OBJECT
    // 信号绑定
    Q_PROPERTY(float temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(float moisture READ moisture NOTIFY moistureChanged)
    Q_PROPERTY(float illumination READ illumination NOTIFY illuminationChanged)
    Q_PROPERTY(bool person READ person NOTIFY personChanged)

public:
    // 外部访问函数
    static Sensor* instance();
    float temperature() const;
    float moisture() const;
    float illumination() const;
    bool person() const;
    void update(float temp, float mois, float illum, bool persona);

signals:
    // 信号
    void temperatureChanged();
    void moistureChanged();
    void illuminationChanged();
    void personChanged();

private:
    float m_temperature;          // 温度
    float m_moisture;             // 预留
    float m_illumination;         // 亮度
    bool m_person;                // true表示有人，false表示无人
    explicit Sensor(QObject *parent = nullptr);
};

#endif // SENSOR_H
// 不能把头文件endif的备注删除！！！
