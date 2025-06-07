#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "sensor.h"
#include "controller.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    Controller controller;

    engine.rootContext()->setContextProperty("sensor", Sensor::instance());

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
            return -1;

    return app.exec();
}
