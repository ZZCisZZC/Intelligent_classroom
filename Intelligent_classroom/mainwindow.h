#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include "sensor.h"
#include "controller.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onTemperatureChanged(float temp);
    void onMoistureChanged(float moist);
    void onIlluminationChanged(float illum);
    void onPersonChanged(bool person);
    void onLightButtonClicked(int index);

private:
    Ui::MainWindow *ui;
    QLabel *m_temp;
    QLabel *m_mois;
    QLabel *m_illum;
    QLabel *m_person;
    QPushButton *m_lights[4];

    Controller *controller;
};

#endif // MAINWINDOW_H
