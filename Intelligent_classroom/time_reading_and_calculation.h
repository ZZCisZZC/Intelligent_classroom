#pragma once
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QString>

class Time_Reading_and_Calculation : public QObject
{
    Q_OBJECT
public:
    explicit Time_Reading_and_Calculation(QObject* parent = nullptr);

    QDateTime getTime() const { return time; }
    void setTime(const QDateTime& t);

signals:
    void timeChanged(const QDateTime&);

private slots:
    void onTick();

private:
    void readTimeFromFile();
    void writeTimeToFile();

    QDateTime time;
    QTimer* timer;
    int tickCount = 0;
    QString confPath = "time.conf";
};