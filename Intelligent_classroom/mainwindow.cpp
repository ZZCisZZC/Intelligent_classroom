#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDialog>
#include <QCheckBox>
#include <QComboBox>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    QWidget* central = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;

    // 添加模式控制
    m_auto = new QPushButton(QString("自动模式 (关)"),this);
    layout->addWidget(m_auto);
    connect(m_auto, &QPushButton::clicked, this, [=]() {onAutoButtonClicked();});

    // 添加面板组件
    m_temp = new QLabel("温度: 0 °C");
    m_mois = new QLabel("湿度: 0 %");
    m_illum = new QLabel("亮度: 0");
    m_person = new QLabel("当前无人");

    layout->addWidget(m_temp);
    layout->addWidget(m_mois);
    layout->addWidget(m_illum);
    layout->addWidget(m_person);

    // 添加灯按钮组件
    for(int i = 0; i < 4; ++i){
         m_lights[i] = new QPushButton(QString("灯%1 (关)").arg(i + 1), this);
         layout->addWidget(m_lights[i]);
         connect(m_lights[i], &QPushButton::clicked, this, [=]() {onLightButtonClicked(i);});
    }
    for (int i = 0; i < 4; ++i) {
        connect(Sensor::instance(), &Sensor::lightStateChanged, this, [=](int index, bool state) {
            if (index == i) {
                m_lights[i]->setText(QString("灯%1 (%2)").arg(index + 1).arg(state ? "开" : "关"));
            }
        });
    }

    // 添加空调控制组件
    m_acStatus = new QLabel("空调状态：关闭 空调模式：制冷 空调挡位：1");
    m_acSettingButton = new QPushButton("空调设置");

    layout->addWidget(m_acStatus);
    layout->addWidget(m_acSettingButton);

    connect(Sensor::instance(), &Sensor::airconditionerStateChanged, this, &MainWindow::onAirConditionerStateChanged);
    connect(Sensor::instance(), &Sensor::airconditionerModeChanged, this, &MainWindow::onAirConditionerModeChanged);
    connect(Sensor::instance(), &Sensor::airconditionerSetChanged, this, &MainWindow::onAirConditionerSetChanged);
    connect(m_acSettingButton, &QPushButton::clicked, this, &MainWindow::openAirConditionerDialog);

    central->setLayout(layout);
    setCentralWidget(central);

    resize(1024,600);

    controller = new Controller(this);

    connect(Sensor::instance(), &Sensor::temperatureChanged, this, &MainWindow::onTemperatureChanged);
    connect(Sensor::instance(), &Sensor::moistureChanged, this, &MainWindow::onMoistureChanged);
    connect(Sensor::instance(), &Sensor::illuminationChanged, this, &MainWindow::onIlluminationChanged);
    connect(Sensor::instance(), &Sensor::personChanged, this, &MainWindow::onPersonChanged);
}

void MainWindow::onAutoButtonClicked() {
    bool n_automode = Sensor::instance()->automode();
    n_automode = !n_automode;
    m_auto->setText(QString("自动模式 (%1)").arg(n_automode ? "开" : "关"));
    Sensor::instance()->updateautomode(n_automode);
}

void MainWindow::onTemperatureChanged(float temp) {
    m_temp->setText(QString("温度: %1 °C").arg(temp, 0, 'f', 1));
}
void MainWindow::onMoistureChanged(float moist) {
    m_mois->setText(QString("湿度: %1 %").arg(moist, 0, 'f', 1));
}
void MainWindow::onIlluminationChanged(float illum) {
    m_illum->setText(QString("亮度: %1").arg(illum, 0, 'f', 0));
}
void MainWindow::onPersonChanged(bool person) {
    m_person->setText(person ? "当前有人" : "当前无人");
}
void MainWindow::onLightButtonClicked(int index) {
    bool n_lightstate = Sensor::instance()->lightstate(index);

    n_lightstate = !n_lightstate;
    int result = controller->getLightState(index, n_lightstate);
    if(result == -1) {
        qDebug() << "操作失败！";
    }
    else{
        //m_lights[index]->setText(QString("灯%1 (%2)").arg(index + 1).arg(n_lightstate ? "开" : "关"));
        Sensor::instance()->updatalightstate(n_lightstate, index);
    }
}
void MainWindow::openAirConditionerDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("空调设置");
    QVBoxLayout* aclayout = new QVBoxLayout;

    QCheckBox* acPower = new QCheckBox("开启空调");
    acPower->setChecked(Sensor::instance()->airconditionerstate());
    aclayout->addWidget(acPower);

    QComboBox* acMode = new QComboBox;
    acMode->addItems({"制冷", "制热"});
    acMode->setCurrentIndex(Sensor::instance()->airconditionermode());
    aclayout->addWidget(acMode);

    QComboBox* acSet = new QComboBox;
    acSet->addItems({"1", "2", "3"});
    acSet->setCurrentIndex(Sensor::instance()->airconditionerset()-1);
    aclayout->addWidget(acSet);

    QPushButton* okBtn = new QPushButton("应用设置");
    aclayout->addWidget(okBtn);

    dialog.setLayout(aclayout);
    connect(okBtn, &QPushButton::clicked, [&]() {
            controller->controlAirConditioner(acPower->isChecked(), acMode->currentIndex(), acSet->currentIndex()+1);
            dialog.accept();
        });
    dialog.exec();
}
void MainWindow::onAirConditionerStateChanged(bool state) {
    m_acStatus->setText(QString("空调状态：%1 空调模式：%2 空调挡位：%3").arg(state ? "开启" : "关闭")
                        .arg((Sensor::instance()->airconditionermode()) ? "制冷" : "制热").arg(Sensor::instance()->airconditionerset()));
}
void MainWindow::onAirConditionerModeChanged(int mode) {
    m_acStatus->setText(QString("空调状态：%1 空调模式：%2 空调挡位：%3").arg((Sensor::instance()->airconditionerstate()) ? "开启" : "关闭")
                        .arg(mode ? "制冷" : "制热").arg(Sensor::instance()->airconditionerset()));
    qDebug() << "模式更新：" << (mode ? "制冷" : "制热");
}
void MainWindow::onAirConditionerSetChanged(int set) {
    m_acStatus->setText(QString("空调状态：%1 空调模式：%2 空调挡位：%3").arg((Sensor::instance()->airconditionerstate()) ? "开启" : "关闭")
                        .arg((Sensor::instance()->airconditionermode()) ? "制冷" : "制热").arg(set));
    qDebug() << "挡位更新：" << set;
}

MainWindow::~MainWindow()
{
}
