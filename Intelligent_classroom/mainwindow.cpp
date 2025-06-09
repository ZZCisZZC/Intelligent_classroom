#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mqttcloud.h"
#include <QDebug>
#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    QWidget* central = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(30);

    QGroupBox* sensorGroup = new QGroupBox("传感器数据");
    sensorGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout* sensorLayout = new QVBoxLayout;

    m_time = new QLabel("时间：0000-00-00 00:00");
    m_temp = new QLabel("温度: 0 °C");
    m_mois = new QLabel("湿度: 0 %");
    m_illum = new QLabel("亮度: 0");
    m_person = new QLabel("当前无人");

    sensorLayout->addWidget(m_time);
    sensorLayout->addWidget(m_temp);
    sensorLayout->addWidget(m_mois);
    sensorLayout->addWidget(m_illum);
    sensorLayout->addWidget(m_person);

    sensorGroup->setLayout(sensorLayout);
    layout->addWidget(sensorGroup);

    QGroupBox* controlGroup = new QGroupBox("控制区域");
    controlGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QVBoxLayout* controlLayout = new QVBoxLayout;

    m_auto = new QPushButton(QString("自动模式 (关)"),this);
    controlLayout->addWidget(m_auto);
    connect(m_auto, &QPushButton::clicked, this, [=]() {onAutoButtonClicked();});

    for(int i = 0; i < 4; ++i){
         m_lights[i] = new QPushButton(QString("灯%1 (关)").arg(i + 1), this);
         controlLayout->addWidget(m_lights[i]);
         connect(m_lights[i], &QPushButton::clicked, this, [=]() {onLightButtonClicked(i);});
         controller->getLightState(i, false);
    }

    m_acStatus = new QLabel("空调状态：关闭 空调模式：制冷 空调挡位：1");
    m_acSettingButton = new QPushButton("空调设置");
    controlLayout->addWidget(m_acStatus);
    controlLayout->addWidget(m_acSettingButton);
    connect(m_acSettingButton, &QPushButton::clicked, this, &MainWindow::openAirConditionerDialog);

    m_multimedia = new QPushButton(QString("多媒体 (关)"));
    controlLayout->addWidget(m_multimedia);
    connect(m_multimedia, &QPushButton::clicked, this, &MainWindow::onMultimediaButtonClicked);

    controlGroup->setLayout(controlLayout);
    layout->addWidget(controlGroup);

    central->setLayout(layout);
    setCentralWidget(central);

    resize(1024,600);
    central->setLayout(layout);
    setCentralWidget(central);

    controller = new Controller(this);
    
    // 启动串口监听
    startSerialListener(controller);

    connect(Sensor::instance(), &Sensor::temperatureChanged, this, &MainWindow::onTemperatureChanged);
    connect(Sensor::instance(), &Sensor::moistureChanged, this, &MainWindow::onMoistureChanged);
    connect(Sensor::instance(), &Sensor::illuminationChanged, this, &MainWindow::onIlluminationChanged);
    connect(Sensor::instance(), &Sensor::personChanged, this, &MainWindow::onPersonChanged);

    connect(Sensor::instance(), &Sensor::airconditionerStateChanged, this, &MainWindow::onAirConditionerStateChanged);
    connect(Sensor::instance(), &Sensor::airconditionerModeChanged, this, &MainWindow::onAirConditionerModeChanged);
    connect(Sensor::instance(), &Sensor::airconditionerSetChanged, this, &MainWindow::onAirConditionerSetChanged);

    connect(Sensor::instance(), &Sensor::multimediaModeChanged, this, &MainWindow::onMultimediaChanged);

    for (int i = 0; i < 4; ++i) {
        connect(Sensor::instance(), &Sensor::lightStateChanged, this, [=](int index, bool state) {
            if (index == i) {
                m_lights[i]->setText(QString("灯%1 (%2)").arg(index + 1).arg(state ? "开" : "关"));
            }
        });
    }

    connect(Sensor::instance(), &Sensor::timeChanged, this, &MainWindow::onTimeChanged);

    this->setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei";
            background-color: #f9f9f9;
        }

        QGroupBox {
            background-color: #ffffff;
            border: 2px solid #39c5bb;
            border-radius: 12px;
            margin-top: 16px;
            padding: 12px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 8px;
            color: #39c5bb;
            font-size: 16px;
            font-weight: bold;
        }

        QLabel {
            font-size: 14px;
            color: #333333;
            background-color: #D3D3D3;
            border-radius: 8px;
            padding: 4px;
        }

        QPushButton {
            background-color: #39c5bb;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14px;
        }

        QPushButton:hover {
            background-color: #2db2a8;
        }

        QPushButton:pressed {
            background-color: #269b93;
        }

        QComboBox, QCheckBox {
            font-size: 14px;
            color: #333333;
        }

        QDialog {
            background-color: #ffffff;
        }
    )");

}

void MainWindow::onAutoButtonClicked() {
    bool n_automode = Sensor::instance()->automode();
    n_automode = !n_automode;
    m_auto->setText(QString("自动模式 (%1)").arg(n_automode ? "开" : "关"));
    Sensor::instance()->updateautomode(n_automode);
    
    // 模式切换时重新应用当前多媒体状态的控制逻辑（不改变状态，只是重新设置定时器逻辑）
    qDebug() << "模式切换: " << (n_automode ? "手动->自动" : "自动->手动") << " 多媒体状态保持: " << Sensor::instance()->multimediamode();
    controller->controlMultiMedia(Sensor::instance()->multimediamode());
}

void MainWindow::onTemperatureChanged(float temp) {
    m_temp->setText(QString("温度: %1 °C").arg(temp, 0, 'f', 1));
}
void MainWindow::onMoistureChanged(float moist) {
    m_mois->setText(QString("湿度: %1 %").arg(moist, 0, 'f', 1));
}
void MainWindow::onIlluminationChanged(float illum) {
    m_illum->setText(QString("亮度: %1").arg(illum, 0, 'f', 2));
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
    dialog.setFixedSize(300, 200);
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
                        .arg((Sensor::instance()->airconditionermode()) ? "制热" : "制冷").arg(Sensor::instance()->airconditionerset()));
}
void MainWindow::onAirConditionerModeChanged(int mode) {
    m_acStatus->setText(QString("空调状态：%1 空调模式：%2 空调挡位：%3").arg((Sensor::instance()->airconditionerstate()) ? "开启" : "关闭")
                        .arg(mode ? "制热" : "制冷").arg(Sensor::instance()->airconditionerset()));
    qDebug() << "模式更新：" << (mode ? "制热" : "制冷");
}
void MainWindow::onAirConditionerSetChanged(int set) {
    m_acStatus->setText(QString("空调状态：%1 空调模式：%2 空调挡位：%3").arg((Sensor::instance()->airconditionerstate()) ? "开启" : "关闭")
                        .arg((Sensor::instance()->airconditionermode()) ? "制热" : "制冷").arg(set));
    qDebug() << "挡位更新：" << set;
}
void MainWindow::onMultimediaChanged(int mode) {
    QString n_mode;
    if ( mode == 0 ) n_mode = "关闭";
    else if ( mode == 1) n_mode = "开启";
    else n_mode = "睡眠";
    m_multimedia->setText(QString("多媒体 (%1)").arg(n_mode));
}
void MainWindow::onMultimediaButtonClicked() {
    int n_mode = Sensor::instance()->multimediamode();
    bool autoMode = Sensor::instance()->automode();
    bool hasPerson = Sensor::instance()->person();
    
    qDebug() << "多媒体按钮点击: 当前模式=" << n_mode << " 自动模式=" << autoMode << " 有人=" << hasPerson;
    
    if (!autoMode) {
        // 手动模式：简单的开关切换
        if ( n_mode == 0) { // 关闭 -> 开启
            n_mode = 1;
        }
        else if ( n_mode == 1) { // 开启 -> 关闭
            n_mode = 0;
        }
        else { // 睡眠 -> 开启
            n_mode = 1;
        }
        qDebug() << "手动模式切换到: " << n_mode;
    } else {
        // 自动模式下的智能切换
        if (hasPerson) {
            // 有人时的切换逻辑
            if (n_mode == 0) { // 关闭 -> 开启
                n_mode = 1;
            } else if (n_mode == 1) { // 开启 -> 关闭
                n_mode = 0;
            } else { // 睡眠 -> 开启（用户明确要使用）
                n_mode = 1;
            }
            qDebug() << "自动模式(有人)切换到: " << n_mode;
        } else {
            // 无人时也允许手动控制，但之后会按自动逻辑运行
            if ( n_mode == 0) { // 关闭 -> 开启
                n_mode = 1;
            }
            else if ( n_mode == 1) { // 开启 -> 关闭
                n_mode = 0;
            }
            else { // 睡眠 -> 开启
                n_mode = 1;
            }
            qDebug() << "自动模式(无人)切换到: " << n_mode << " (将根据自动逻辑继续管理)";
        }
    }
    
    controller->controlMultiMedia(n_mode);
}
void MainWindow::onTimeChanged(int year, int month, int day, int hour, int minute) {
    QString timeText = QString("时间：%1-%2-%3 %4:%5")
        .arg(year, 4, 10, QLatin1Char('0'))
        .arg(month, 2, 10, QLatin1Char('0'))
        .arg(day, 2, 10, QLatin1Char('0'))
        .arg(hour, 2, 10, QLatin1Char('0'))
        .arg(minute, 2, 10, QLatin1Char('0'));

    m_time->setText(timeText);
}

MainWindow::~MainWindow()
{
}
