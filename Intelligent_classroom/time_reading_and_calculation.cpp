#include "time_reading_and_calculation.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

Time_Reading_and_Calculation::Time_Reading_and_Calculation(QObject* parent)
    : QObject(parent), timer(new QTimer(this))
{
    readTimeFromFile(); // 启动时先从文件读取时间
    connect(timer, &QTimer::timeout, this, &Time_Reading_and_Calculation::onTick); // 定时器超时信号连接到onTick槽
    timer->start(1000); // 每1秒触发一次
}

void Time_Reading_and_Calculation::setTime(const QDateTime& t)
{
    time = t;             // 设置当前时间
    writeTimeToFile();    // 写入文件
    emit timeChanged(time); // 发出时间变化信号
}

void Time_Reading_and_Calculation::onTick()
{
    tickCount++;              // 计数器+1
    time = time.addSecs(600); // 每1秒加10分钟（600秒）
    
    // if (tickCount % 6 == 0) {
    //     time = time.addSecs(3600); // 加1小时（3600秒）
    // }
    writeTimeToFile();        // 写入文件
    emit timeChanged(time);   // 发出时间变化信号
}

void Time_Reading_and_Calculation::readTimeFromFile()
{
    QFile file(confPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) { // 打开配置文件
        QTextStream in(&file);
        QString line = in.readLine(); // 读取一行
        QDateTime t = QDateTime::fromString(line, Qt::ISODate); // 按ISO格式解析时间
        if (t.isValid()) {
            time = t; // 解析成功则赋值
        } else {
            time = QDateTime::currentDateTime(); // 失败则用当前时间
        }
        file.close();
    } else {
        time = QDateTime::currentDateTime(); // 文件打不开也用当前时间
    }
}

void Time_Reading_and_Calculation::writeTimeToFile()
{
    QFile file(confPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) { // 打开文件准备写入
        QTextStream out(&file);
        out << time.toString(Qt::ISODate) << "\n"; // 以ISO格式写入时间
        file.close();
    }
}
//Qt ISO 时间格式为yyyy-MM-ddTHH:mm:ss 这样的字符串，即：
//2025-06-08T14:30:00