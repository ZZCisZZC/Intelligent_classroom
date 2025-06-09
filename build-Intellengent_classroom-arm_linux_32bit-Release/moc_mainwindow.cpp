/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Intelligent_classroom/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[26];
    char stringdata0[361];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 19), // "onAutoButtonClicked"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 20), // "onTemperatureChanged"
QT_MOC_LITERAL(4, 53, 4), // "temp"
QT_MOC_LITERAL(5, 58, 17), // "onMoistureChanged"
QT_MOC_LITERAL(6, 76, 5), // "moist"
QT_MOC_LITERAL(7, 82, 21), // "onIlluminationChanged"
QT_MOC_LITERAL(8, 104, 5), // "illum"
QT_MOC_LITERAL(9, 110, 15), // "onPersonChanged"
QT_MOC_LITERAL(10, 126, 6), // "person"
QT_MOC_LITERAL(11, 133, 20), // "onLightButtonClicked"
QT_MOC_LITERAL(12, 154, 5), // "index"
QT_MOC_LITERAL(13, 160, 28), // "onAirConditionerStateChanged"
QT_MOC_LITERAL(14, 189, 27), // "onAirConditionerModeChanged"
QT_MOC_LITERAL(15, 217, 26), // "onAirConditionerSetChanged"
QT_MOC_LITERAL(16, 244, 24), // "openAirConditionerDialog"
QT_MOC_LITERAL(17, 269, 19), // "onMultimediaChanged"
QT_MOC_LITERAL(18, 289, 4), // "mode"
QT_MOC_LITERAL(19, 294, 25), // "onMultimediaButtonClicked"
QT_MOC_LITERAL(20, 320, 13), // "onTimeChanged"
QT_MOC_LITERAL(21, 334, 4), // "year"
QT_MOC_LITERAL(22, 339, 5), // "month"
QT_MOC_LITERAL(23, 345, 3), // "day"
QT_MOC_LITERAL(24, 349, 4), // "hour"
QT_MOC_LITERAL(25, 354, 6) // "minute"

    },
    "MainWindow\0onAutoButtonClicked\0\0"
    "onTemperatureChanged\0temp\0onMoistureChanged\0"
    "moist\0onIlluminationChanged\0illum\0"
    "onPersonChanged\0person\0onLightButtonClicked\0"
    "index\0onAirConditionerStateChanged\0"
    "onAirConditionerModeChanged\0"
    "onAirConditionerSetChanged\0"
    "openAirConditionerDialog\0onMultimediaChanged\0"
    "mode\0onMultimediaButtonClicked\0"
    "onTimeChanged\0year\0month\0day\0hour\0"
    "minute"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    1,   80,    2, 0x08 /* Private */,
       5,    1,   83,    2, 0x08 /* Private */,
       7,    1,   86,    2, 0x08 /* Private */,
       9,    1,   89,    2, 0x08 /* Private */,
      11,    1,   92,    2, 0x08 /* Private */,
      13,    1,   95,    2, 0x08 /* Private */,
      14,    1,   98,    2, 0x08 /* Private */,
      15,    1,  101,    2, 0x08 /* Private */,
      16,    0,  104,    2, 0x08 /* Private */,
      17,    1,  105,    2, 0x08 /* Private */,
      19,    0,  108,    2, 0x08 /* Private */,
      20,    5,  109,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float,    4,
    QMetaType::Void, QMetaType::Float,    6,
    QMetaType::Void, QMetaType::Float,    8,
    QMetaType::Void, QMetaType::Bool,   10,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   18,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,   21,   22,   23,   24,   25,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onAutoButtonClicked(); break;
        case 1: _t->onTemperatureChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: _t->onMoistureChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 3: _t->onIlluminationChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 4: _t->onPersonChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->onLightButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->onAirConditionerStateChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->onAirConditionerModeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->onAirConditionerSetChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->openAirConditionerDialog(); break;
        case 10: _t->onMultimediaChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->onMultimediaButtonClicked(); break;
        case 12: _t->onTimeChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5]))); break;
        default: ;
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
