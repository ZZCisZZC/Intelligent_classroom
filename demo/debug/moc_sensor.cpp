/****************************************************************************
** Meta object code from reading C++ file 'sensor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../test/sensor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sensor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Sensor_t {
    QByteArrayData data[10];
    char stringdata0[118];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Sensor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Sensor_t qt_meta_stringdata_Sensor = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Sensor"
QT_MOC_LITERAL(1, 7, 18), // "temperatureChanged"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 15), // "moistureChanged"
QT_MOC_LITERAL(4, 43, 19), // "illuminationChanged"
QT_MOC_LITERAL(5, 63, 13), // "personChanged"
QT_MOC_LITERAL(6, 77, 11), // "temperature"
QT_MOC_LITERAL(7, 89, 8), // "moisture"
QT_MOC_LITERAL(8, 98, 12), // "illumination"
QT_MOC_LITERAL(9, 111, 6) // "person"

    },
    "Sensor\0temperatureChanged\0\0moistureChanged\0"
    "illuminationChanged\0personChanged\0"
    "temperature\0moisture\0illumination\0"
    "person"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Sensor[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       4,   38, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    0,   35,    2, 0x06 /* Public */,
       4,    0,   36,    2, 0x06 /* Public */,
       5,    0,   37,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       6, QMetaType::Float, 0x00495001,
       7, QMetaType::Float, 0x00495001,
       8, QMetaType::Float, 0x00495001,
       9, QMetaType::Bool, 0x00495001,

 // properties: notify_signal_id
       0,
       1,
       2,
       3,

       0        // eod
};

void Sensor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Sensor *_t = static_cast<Sensor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->temperatureChanged(); break;
        case 1: _t->moistureChanged(); break;
        case 2: _t->illuminationChanged(); break;
        case 3: _t->personChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Sensor::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::temperatureChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::moistureChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::illuminationChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::personChanged)) {
                *result = 3;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        Sensor *_t = static_cast<Sensor *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< float*>(_v) = _t->temperature(); break;
        case 1: *reinterpret_cast< float*>(_v) = _t->moisture(); break;
        case 2: *reinterpret_cast< float*>(_v) = _t->illumination(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->person(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
    Q_UNUSED(_a);
}

const QMetaObject Sensor::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Sensor.data,
      qt_meta_data_Sensor,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Sensor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Sensor::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Sensor.stringdata0))
        return static_cast<void*>(const_cast< Sensor*>(this));
    return QObject::qt_metacast(_clname);
}

int Sensor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void Sensor::temperatureChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void Sensor::moistureChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void Sensor::illuminationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void Sensor::personChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
