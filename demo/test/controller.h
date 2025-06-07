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

private slots:
    void GetData();

private:
    QTimer *timer;

};

#endif // CONTROLLER_H
// 不能把头文件endif的备注删除！！！
