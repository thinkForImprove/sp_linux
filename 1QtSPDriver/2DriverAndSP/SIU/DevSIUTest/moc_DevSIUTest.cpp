/****************************************************************************
** Meta object code from reading C++ file 'DevSIUTest.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DevSIUTest.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DevSIUTest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CDevSIUTest_t {
    QByteArrayData data[15];
    char stringdata0[354];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDevSIUTest_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDevSIUTest_t qt_meta_stringdata_CDevSIUTest = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CDevSIUTest"
QT_MOC_LITERAL(1, 12, 19), // "on_pbt_Open_clicked"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 20), // "on_pbt_Close_clicked"
QT_MOC_LITERAL(4, 54, 20), // "on_pbt_Reset_clicked"
QT_MOC_LITERAL(5, 75, 25), // "on_pbt_GetDevInfo_clicked"
QT_MOC_LITERAL(6, 101, 24), // "on_pbt_GetStatus_clicked"
QT_MOC_LITERAL(7, 126, 23), // "on_pbt_SetDoors_clicked"
QT_MOC_LITERAL(8, 150, 28), // "on_pbt_SetIndicators_clicked"
QT_MOC_LITERAL(9, 179, 29), // "on_pbt_SetAuxiliaries_clicked"
QT_MOC_LITERAL(10, 209, 28), // "on_pbt_SetGuidLights_clicked"
QT_MOC_LITERAL(11, 238, 29), // "on_pbt_GetFirmWareVer_clicked"
QT_MOC_LITERAL(12, 268, 27), // "on_pbt_UpdateDevPDL_clicked"
QT_MOC_LITERAL(13, 296, 34), // "on_pbt_SetGuidLights_Close_cl..."
QT_MOC_LITERAL(14, 331, 22) // "on_pbtClearLog_clicked"

    },
    "CDevSIUTest\0on_pbt_Open_clicked\0\0"
    "on_pbt_Close_clicked\0on_pbt_Reset_clicked\0"
    "on_pbt_GetDevInfo_clicked\0"
    "on_pbt_GetStatus_clicked\0"
    "on_pbt_SetDoors_clicked\0"
    "on_pbt_SetIndicators_clicked\0"
    "on_pbt_SetAuxiliaries_clicked\0"
    "on_pbt_SetGuidLights_clicked\0"
    "on_pbt_GetFirmWareVer_clicked\0"
    "on_pbt_UpdateDevPDL_clicked\0"
    "on_pbt_SetGuidLights_Close_clicked\0"
    "on_pbtClearLog_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDevSIUTest[] = {

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
       3,    0,   80,    2, 0x08 /* Private */,
       4,    0,   81,    2, 0x08 /* Private */,
       5,    0,   82,    2, 0x08 /* Private */,
       6,    0,   83,    2, 0x08 /* Private */,
       7,    0,   84,    2, 0x08 /* Private */,
       8,    0,   85,    2, 0x08 /* Private */,
       9,    0,   86,    2, 0x08 /* Private */,
      10,    0,   87,    2, 0x08 /* Private */,
      11,    0,   88,    2, 0x08 /* Private */,
      12,    0,   89,    2, 0x08 /* Private */,
      13,    0,   90,    2, 0x08 /* Private */,
      14,    0,   91,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CDevSIUTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CDevSIUTest *_t = static_cast<CDevSIUTest *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_pbt_Open_clicked(); break;
        case 1: _t->on_pbt_Close_clicked(); break;
        case 2: _t->on_pbt_Reset_clicked(); break;
        case 3: _t->on_pbt_GetDevInfo_clicked(); break;
        case 4: _t->on_pbt_GetStatus_clicked(); break;
        case 5: _t->on_pbt_SetDoors_clicked(); break;
        case 6: _t->on_pbt_SetIndicators_clicked(); break;
        case 7: _t->on_pbt_SetAuxiliaries_clicked(); break;
        case 8: _t->on_pbt_SetGuidLights_clicked(); break;
        case 9: _t->on_pbt_GetFirmWareVer_clicked(); break;
        case 10: _t->on_pbt_UpdateDevPDL_clicked(); break;
        case 11: _t->on_pbt_SetGuidLights_Close_clicked(); break;
        case 12: _t->on_pbtClearLog_clicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CDevSIUTest::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CDevSIUTest.data,
      qt_meta_data_CDevSIUTest,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *CDevSIUTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDevSIUTest::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_CDevSIUTest.stringdata0))
        return static_cast<void*>(const_cast< CDevSIUTest*>(this));
    return QWidget::qt_metacast(_clname);
}

int CDevSIUTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
