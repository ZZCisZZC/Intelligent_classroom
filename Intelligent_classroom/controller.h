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

signals:
    void startSleepTimer();
    void startOffTimer();

public slots:
    void generalControl();                                          // 总控
    void getSensorData();                                           // 从硬件获取数据
    int getLightState(int lightNum,bool data);                      // 控制灯状态
    void controlAirConditioner(bool state, int mode=0, int set=1);  // 控制空调状态
    void controlMultiMedia(int mode);                               // 控制多媒体模式
    void uploadData();
    void setControl(std::string jsonStr);
    void getTimeFromHardware();
    void timeHandler();
    void uploadTimeToHardware();
    void onStartSleepTimer();    // 响应启动睡眠定时器信号
    void onStartOffTimer();      // 响应启动关闭定时器信号

private:
    QTimer *timer;
    QTimer* m_sleepTimer;
    QTimer* m_offTimer;
    QTimer* m_upload;
    QTimer* m_clock;
    QTimer* m_timetofile;

};

#endif // CONTROLLER_H
// 不能把头文件endif的备注删除！！！
