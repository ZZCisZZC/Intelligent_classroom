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
    QByteArrayData data[19];
    char stringdata0[219];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Controller_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Controller_t qt_meta_stringdata_Controller = {
    {
QT_MOC_LITERAL(0, 0, 10), // "Controller"
QT_MOC_LITERAL(1, 11, 14), // "generalControl"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 13), // "getSensorData"
QT_MOC_LITERAL(4, 41, 13), // "getLightState"
QT_MOC_LITERAL(5, 55, 8), // "lightNum"
QT_MOC_LITERAL(6, 64, 4), // "data"
QT_MOC_LITERAL(7, 69, 21), // "controlAirConditioner"
QT_MOC_LITERAL(8, 91, 5), // "state"
QT_MOC_LITERAL(9, 97, 4), // "mode"
QT_MOC_LITERAL(10, 102, 3), // "set"
QT_MOC_LITERAL(11, 106, 17), // "controlMultiMedia"
QT_MOC_LITERAL(12, 124, 10), // "uploadData"
QT_MOC_LITERAL(13, 135, 10), // "setControl"
QT_MOC_LITERAL(14, 146, 11), // "std::string"
QT_MOC_LITERAL(15, 158, 7), // "jsonStr"
QT_MOC_LITERAL(16, 166, 19), // "getTimeFromHardware"
QT_MOC_LITERAL(17, 186, 11), // "timeHandler"
QT_MOC_LITERAL(18, 198, 20) // "uploadTimeToHardware"

    },
    "Controller\0generalControl\0\0getSensorData\0"
    "getLightState\0lightNum\0data\0"
    "controlAirConditioner\0state\0mode\0set\0"
    "controlMultiMedia\0uploadData\0setControl\0"
    "std::string\0jsonStr\0getTimeFromHardware\0"
    "timeHandler\0uploadTimeToHardware"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Controller[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x0a /* Public */,
       3,    0,   75,    2, 0x0a /* Public */,
       4,    2,   76,    2, 0x0a /* Public */,
       7,    3,   81,    2, 0x0a /* Public */,
       7,    2,   88,    2, 0x2a /* Public | MethodCloned */,
       7,    1,   93,    2, 0x2a /* Public | MethodCloned */,
      11,    1,   96,    2, 0x0a /* Public */,
      12,    0,   99,    2, 0x0a /* Public */,
      13,    1,  100,    2, 0x0a /* Public */,
      16,    0,  103,    2, 0x0a /* Public */,
      17,    0,  104,    2, 0x0a /* Public */,
      18,    0,  105,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Int, QMetaType::Int, QMetaType::Bool,    5,    6,
    QMetaType::Void, QMetaType::Bool, QMetaType::Int, QMetaType::Int,    8,    9,   10,
    QMetaType::Void, QMetaType::Bool, QMetaType::Int,    8,    9,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 14,   15,
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
        case 0: _t->generalControl(); break;
        case 1: _t->getSensorData(); break;
        case 2: { int _r = _t->getLightState((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 3: _t->controlAirConditioner((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 4: _t->controlAirConditioner((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->controlAirConditioner((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->controlMultiMedia((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->uploadData(); break;
        case 8: _t->setControl((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 9: _t->getTimeFromHardware(); break;
        case 10: _t->timeHandler(); break;
        case 11: _t->uploadTimeToHardware(); break;
        default: ;
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
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
