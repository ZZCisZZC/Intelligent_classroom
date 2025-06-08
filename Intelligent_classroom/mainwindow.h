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
    void onAutoButtonClicked();
    void onTemperatureChanged(float temp);
    void onMoistureChanged(float moist);
    void onIlluminationChanged(float illum);
    void onPersonChanged(bool person);
    void onLightButtonClicked(int index);
    void onAirConditionerStateChanged(bool);
    void onAirConditionerModeChanged(int);
    void onAirConditionerSetChanged(int);
    void openAirConditionerDialog();
    void onMultimediaChanged(int mode);
    void onMultimediaButtonClicked();

private:
    Ui::MainWindow *ui;
    QPushButton *m_auto;
    QLabel *m_temp;
    QLabel *m_mois;
    QLabel *m_illum;
    QLabel *m_person;
    QPushButton *m_lights[4];
    QLabel* m_acStatus;
    QPushButton* m_acSettingButton;
    QPushButton* m_multimedia;

    Controller *controller;
};

#endif // MAINWINDOW_H
