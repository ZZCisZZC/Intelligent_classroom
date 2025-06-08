#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    QWidget* central = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;

    // 添加面板组件
    m_temp = new QLabel("温度: 0 °C");
    m_mois = new QLabel("湿度: 0 %");
    m_illum = new QLabel("亮度: 0");
    m_person = new QLabel("当前无人");

    layout->addWidget(m_temp);
    layout->addWidget(m_mois);
    layout->addWidget(m_illum);
    layout->addWidget(m_person);

    // 添加按钮组件
    for(int i = 0; i < 4; ++i){
         m_lights[i] = new QPushButton(QString("Light%1 (关)").arg(i + 1), this);
         layout->addWidget(m_lights[i]);
         connect(m_lights[i], &QPushButton::clicked, this, [=]() {onLightButtonClicked(i);});
    }

    central->setLayout(layout);
    setCentralWidget(central);

    resize(1024,600);

    controller = new Controller(this);

    connect(Sensor::instance(), &Sensor::temperatureChanged, this, &MainWindow::onTemperatureChanged);
    connect(Sensor::instance(), &Sensor::moistureChanged, this, &MainWindow::onMoistureChanged);
    connect(Sensor::instance(), &Sensor::illuminationChanged, this, &MainWindow::onIlluminationChanged);
    connect(Sensor::instance(), &Sensor::personChanged, this, &MainWindow::onPersonChanged);
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
        m_lights[index]->setText(QString("Light%1 (%2)").arg(index + 1).arg(n_lightstate ? "开" : "关"));
        Sensor::instance()->updatalightstate(n_lightstate, index);
    }
}


MainWindow::~MainWindow()
{
}
