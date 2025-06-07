#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    QWidget* central = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;

    m_temp = new QLabel("温度: 0 °C");
    m_mois = new QLabel("湿度: 0 %");
    m_illum = new QLabel("亮度: 0");
    m_person = new QLabel("当前无人");

    layout->addWidget(m_temp);
    layout->addWidget(m_mois);
    layout->addWidget(m_illum);
    layout->addWidget(m_person);

    central->setLayout(layout);
    setCentralWidget(central);

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
    m_illum->setText(QString("亮度: %1").arg(illum, 0, 'f', 0));
}
void MainWindow::onPersonChanged(bool person) {
    m_person->setText(person ? "当前有人" : "当前无人");
}


MainWindow::~MainWindow()
{
}
