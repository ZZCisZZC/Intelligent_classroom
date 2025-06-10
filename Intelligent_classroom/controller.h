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
    void startAirConditionerOffTimer();    // 空调自动关闭定时器信号

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
    void onStartAirConditionerOffTimer();  // 响应启动空调关闭定时器信号

private:
    QTimer *timer;
    QTimer* m_sleepTimer;
    QTimer* m_offTimer;
    QTimer* m_airConditionerOffTimer;  // 空调自动关闭定时器
    QTimer* m_upload;
    QTimer* m_clock;
    QTimer* m_timetofile;
    
    // 日期处理辅助函数
    bool isLeapYear(int year);                          // 判断是否为闰年
    int getDaysInMonth(int year, int month);            // 获取指定年月的天数

};

#endif // CONTROLLER_H
// 不能把头文件endif的备注删除！！！
