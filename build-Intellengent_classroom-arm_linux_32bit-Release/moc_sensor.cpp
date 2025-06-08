/****************************************************************************
** Meta object code from reading C++ file 'sensor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Intelligent_classroom/sensor.h"
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
    QByteArrayData data[15];
    char stringdata0[216];
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
QT_MOC_LITERAL(6, 77, 17), // "lightStateChanged"
QT_MOC_LITERAL(7, 95, 5), // "index"
QT_MOC_LITERAL(8, 101, 5), // "state"
QT_MOC_LITERAL(9, 107, 26), // "airconditionerStateChanged"
QT_MOC_LITERAL(10, 134, 25), // "airconditionerModeChanged"
QT_MOC_LITERAL(11, 160, 4), // "mode"
QT_MOC_LITERAL(12, 165, 24), // "airconditionerSetChanged"
QT_MOC_LITERAL(13, 190, 3), // "set"
QT_MOC_LITERAL(14, 194, 21) // "multimediaModeChanged"

    },
    "Sensor\0temperatureChanged\0\0moistureChanged\0"
    "illuminationChanged\0personChanged\0"
    "lightStateChanged\0index\0state\0"
    "airconditionerStateChanged\0"
    "airconditionerModeChanged\0mode\0"
    "airconditionerSetChanged\0set\0"
    "multimediaModeChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Sensor[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       3,    1,   62,    2, 0x06 /* Public */,
       4,    1,   65,    2, 0x06 /* Public */,
       5,    1,   68,    2, 0x06 /* Public */,
       6,    2,   71,    2, 0x06 /* Public */,
       9,    1,   76,    2, 0x06 /* Public */,
      10,    1,   79,    2, 0x06 /* Public */,
      12,    1,   82,    2, 0x06 /* Public */,
      14,    1,   85,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Float,    2,
    QMetaType::Void, QMetaType::Float,    2,
    QMetaType::Void, QMetaType::Float,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    7,    8,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,   11,

       0        // eod
};

void Sensor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Sensor *_t = static_cast<Sensor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->temperatureChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: _t->moistureChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: _t->illuminationChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 3: _t->personChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->lightStateChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->airconditionerStateChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->airconditionerModeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->airconditionerSetChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->multimediaModeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Sensor::*_t)(float );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::temperatureChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)(float );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::moistureChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)(float );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::illuminationChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::personChanged)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)(int , bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::lightStateChanged)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::airconditionerStateChanged)) {
                *result = 5;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::airconditionerModeChanged)) {
                *result = 6;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::airconditionerSetChanged)) {
                *result = 7;
                return;
            }
        }
        {
            typedef void (Sensor::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Sensor::multimediaModeChanged)) {
                *result = 8;
                return;
            }
        }
    }
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
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void Sensor::temperatureChanged(float _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Sensor::moistureChanged(float _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Sensor::illuminationChanged(float _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Sensor::personChanged(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Sensor::lightStateChanged(int _t1, bool _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Sensor::airconditionerStateChanged(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Sensor::airconditionerModeChanged(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void Sensor::airconditionerSetChanged(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void Sensor::multimediaModeChanged(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_END_MOC_NAMESPACE
