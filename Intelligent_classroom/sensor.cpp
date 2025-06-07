#include "sensor.h"

Sensor::Sensor(QObject *parent) : QObject(parent),
    m_temperature(0.0), m_moisture(0.0), m_illumination(0.0), m_person(false) {}

Sensor* Sensor::instance() {
    static Sensor sensorInstance;
    return &sensorInstance;
}

float Sensor::temperature() const { return m_temperature; }
float Sensor::moisture() const { return m_moisture; }
float Sensor::illumination() const { return m_illumination; }
bool Sensor::person() const { return m_person; }
bool Sensor::lightstate(int index) const { return m_lightStates[index]; }

void Sensor::update(float temp, float mois, float illum, bool person) { // 外部更新sensor的值
    if (m_temperature != temp) {
        m_temperature = temp;
        emit temperatureChanged(temp);
    }
    if (m_moisture != mois) {
        m_moisture = mois;
        emit moistureChanged(mois);
    }
    if (m_illumination != illum) {
        m_illumination = illum;
        emit illuminationChanged(illum);
    }
    if (m_person != person) {
        m_person = person;
        emit personChanged(person);
    }
}

void Sensor::updatalightstate(bool state, int index) {
    m_lightStates[index] = state;
}
