/****************************************************************************
** Meta object code from reading C++ file 'controller.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Intelligent_classroom/controller.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'controller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Controller_t {
    QByteArrayData data[25];
    char stringdata0[341];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Controller_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Controller_t qt_meta_stringdata_Controller = {
    {
QT_MOC_LITERAL(0, 0, 10), // "Controller"
QT_MOC_LITERAL(1, 11, 15), // "startSleepTimer"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 13), // "startOffTimer"
QT_MOC_LITERAL(4, 42, 27), // "startAirConditionerOffTimer"
QT_MOC_LITERAL(5, 70, 14), // "generalControl"
QT_MOC_LITERAL(6, 85, 13), // "getSensorData"
QT_MOC_LITERAL(7, 99, 13), // "getLightState"
QT_MOC_LITERAL(8, 113, 8), // "lightNum"
QT_MOC_LITERAL(9, 122, 4), // "data"
QT_MOC_LITERAL(10, 127, 21), // "controlAirConditioner"
QT_MOC_LITERAL(11, 149, 5), // "state"
QT_MOC_LITERAL(12, 155, 4), // "mode"
QT_MOC_LITERAL(13, 160, 3), // "set"
QT_MOC_LITERAL(14, 164, 17), // "controlMultiMedia"
QT_MOC_LITERAL(15, 182, 10), // "uploadData"
QT_MOC_LITERAL(16, 193, 10), // "setControl"
QT_MOC_LITERAL(17, 204, 11), // "std::string"
QT_MOC_LITERAL(18, 216, 7), // "jsonStr"
QT_MOC_LITERAL(19, 224, 19), // "getTimeFromHardware"
QT_MOC_LITERAL(20, 244, 11), // "timeHandler"
QT_MOC_LITERAL(21, 256, 20), // "uploadTimeToHardware"
QT_MOC_LITERAL(22, 277, 17), // "onStartSleepTimer"
QT_MOC_LITERAL(23, 295, 15), // "onStartOffTimer"
QT_MOC_LITERAL(24, 311, 29) // "onStartAirConditionerOffTimer"

    },
    "Controller\0startSleepTimer\0\0startOffTimer\0"
    "startAirConditionerOffTimer\0generalControl\0"
    "getSensorData\0getLightState\0lightNum\0"
    "data\0controlAirConditioner\0state\0mode\0"
    "set\0controlMultiMedia\0uploadData\0"
    "setControl\0std::string\0jsonStr\0"
    "getTimeFromHardware\0timeHandler\0"
    "uploadTimeToHardware\0onStartSleepTimer\0"
    "onStartOffTimer\0onStartAirConditionerOffTimer"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Controller[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  104,    2, 0x06 /* Public */,
       3,    0,  105,    2, 0x06 /* Public */,
       4,    0,  106,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,  107,    2, 0x0a /* Public */,
       6,    0,  108,    2, 0x0a /* Public */,
       7,    2,  109,    2, 0x0a /* Public */,
      10,    3,  114,    2, 0x0a /* Public */,
      10,    2,  121,    2, 0x2a /* Public | MethodCloned */,
      10,    1,  126,    2, 0x2a /* Public | MethodCloned */,
      14,    1,  129,    2, 0x0a /* Public */,
      15,    0,  132,    2, 0x0a /* Public */,
      16,    1,  133,    2, 0x0a /* Public */,
      19,    0,  136,    2, 0x0a /* Public */,
      20,    0,  137,    2, 0x0a /* Public */,
      21,    0,  138,    2, 0x0a /* Public */,
      22,    0,  139,    2, 0x0a /* Public */,
      23,    0,  140,    2, 0x0a /* Public */,
      24,    0,  141,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Int, QMetaType::Int, QMetaType::Bool,    8,    9,
    QMetaType::Void, QMetaType::Bool, QMetaType::Int, QMetaType::Int,   11,   12,   13,
    QMetaType::Void, QMetaType::Bool, QMetaType::Int,   11,   12,
    QMetaType::Void, QMetaType::Bool,   11,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 17,   18,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Controller::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Controller *_t = static_cast<Controller *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->startSleepTimer(); break;
        case 1: _t->startOffTimer(); break;
        case 2: _t->startAirConditionerOffTimer(); break;
        case 3: _t->generalControl(); break;
        case 4: _t->getSensorData(); break;
        case 5: { int _r = _t->getLightState((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 6: _t->controlAirConditioner((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 7: _t->controlAirConditioner((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->controlAirConditioner((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->controlMultiMedia((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->uploadData(); break;
        case 11: _t->setControl((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 12: _t->getTimeFromHardware(); break;
        case 13: _t->timeHandler(); break;
        case 14: _t->uploadTimeToHardware(); break;
        case 15: _t->onStartSleepTimer(); break;
        case 16: _t->onStartOffTimer(); break;
        case 17: _t->onStartAirConditionerOffTimer(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Controller::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Controller::startSleepTimer)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (Controller::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Controller::startOffTimer)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (Controller::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Controller::startAirConditionerOffTimer)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject Controller::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Controller.data,
      qt_meta_data_Controller,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Controller::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Controller::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Controller.stringdata0))
        return static_cast<void*>(const_cast< Controller*>(this));
    return QObject::qt_metacast(_clname);
}

int Controller::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void Controller::startSleepTimer()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void Controller::startOffTimer()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void Controller::startAirConditionerOffTimer()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
