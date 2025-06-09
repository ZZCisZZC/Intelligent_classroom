#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QString>
#include <QObject>
#include <QTimer>
#include <string>

class Controller: public QObject
{
    Q_OBJECT

public:
    Controller(QObject* parent = nullptr);

public slots:
    void generalControl();                                          // 总控
    void getSensorData();                                           // 从硬件获取数据
    int getLightState(int lightNum,bool data);                      // 控制灯状态
    void controlAirConditioner(bool state, int mode=0, int set=1);  // 控制空调状态
    void controlMultiMedia(int mode);                               // 控制多媒体模式
    void uploadData();
    void setControl(std::string jsonStr);

private:
    QTimer *timer;
    QTimer* m_sleepTimer;
    QTimer* m_offTimer;
    QTimer* m_upload;

};

#endif // CONTROLLER_H
// 不能把头文件endif的备注删除！！！
