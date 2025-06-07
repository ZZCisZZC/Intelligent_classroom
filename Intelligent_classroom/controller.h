#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QString>
#include <QObject>
#include <QTimer>

class Controller: public QObject
{
    Q_OBJECT

public:
    Controller(QObject* parent = nullptr);

public slots:
    void getSensorData();                           // 从硬件获取数据
    int getLightState(int lightNum,bool data);      // 控制灯状态

private:
    QTimer *timer;

};

#endif // CONTROLLER_H
// 不能把头文件endif的备注删除！！！
