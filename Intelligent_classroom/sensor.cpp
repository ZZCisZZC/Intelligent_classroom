#include "sensor.h"

Sensor::Sensor(QObject *parent) : QObject(parent),
    m_temperature(0.0), m_moisture(0.0), m_illumination(0.0), m_person(false) {}

Sensor* Sensor::instance() {
    static Sensor sensorInstance;
    return &sensorInstance;
}

bool Sensor::automode() const { return m_automode; }
float Sensor::temperature() const { return m_temperature; }
float Sensor::moisture() const { return m_moisture; }
float Sensor::illumination() const { return m_illumination; }
bool Sensor::person() const { return m_person; }
bool Sensor::lightstate(int index) const { return m_lightStates[index]; }
bool Sensor::airconditionerstate() const { return m_airconditionerState; }
int Sensor::airconditionermode() const { return m_airconditionerMode; }
int Sensor::airconditionerset() const { return m_airconditionerSet; }
int Sensor::multimediamode() const { return m_multimediaMode; }
int Sensor::getyear() const { return m_year; }
int Sensor::getmonth() const { return m_month; }
int Sensor::getday() const { return m_day; }
int Sensor::gethour() const { return m_hour; }
int Sensor::getminute() const { return m_minute; }

void Sensor::updateautomode(bool state) {
    m_automode = state;
}
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
    emit lightStateChanged(index, state);
}
void Sensor::updateairconditioner(bool state, int mode, int set) {
    if (m_airconditionerState != state) {
        m_airconditionerState = state;
        emit airconditionerStateChanged(state);
    }
    if (m_airconditionerMode != mode) {
        m_airconditionerMode = mode;
        emit airconditionerModeChanged(mode);
    }
    if (m_airconditionerSet != set) {
        m_airconditionerSet = set;
        emit airconditionerSetChanged(set);
    }
}
void Sensor::updatemultimediamode(int mode) {
    if (m_multimediaMode != mode) {
        m_multimediaMode = mode;
        emit multimediaModeChanged(mode);
    }
}
