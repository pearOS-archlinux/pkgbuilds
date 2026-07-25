/****************************************************************************
** Meta object code from reading C++ file 'dbusinterface.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/dbusinterface.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dbusinterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DBusInterface_t {
    QByteArrayData data[9];
    char stringdata0[119];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DBusInterface_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DBusInterface_t qt_meta_stringdata_DBusInterface = {
    {
QT_MOC_LITERAL(0, 0, 13), // "DBusInterface"
QT_MOC_LITERAL(1, 14, 15), // "D-Bus Interface"
QT_MOC_LITERAL(2, 30, 28), // "org.freedesktop.FileManager1"
QT_MOC_LITERAL(3, 59, 11), // "ShowFolders"
QT_MOC_LITERAL(4, 71, 0), // ""
QT_MOC_LITERAL(5, 72, 7), // "uriList"
QT_MOC_LITERAL(6, 80, 9), // "startUpId"
QT_MOC_LITERAL(7, 90, 9), // "ShowItems"
QT_MOC_LITERAL(8, 100, 18) // "ShowItemProperties"

    },
    "DBusInterface\0D-Bus Interface\0"
    "org.freedesktop.FileManager1\0ShowFolders\0"
    "\0uriList\0startUpId\0ShowItems\0"
    "ShowItemProperties"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DBusInterface[] = {

 // content:
       8,       // revision
       0,       // classname
       1,   14, // classinfo
       3,   16, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // classinfo: key, value
       1,    2,

 // methods: name, argc, parameters, tag, flags
       3,    2,   31,    4, 0x42 /* Public | isScriptable */,
       7,    2,   36,    4, 0x42 /* Public | isScriptable */,
       8,    2,   41,    4, 0x42 /* Public | isScriptable */,

 // methods: parameters
    QMetaType::Void, QMetaType::QStringList, QMetaType::QString,    5,    6,
    QMetaType::Void, QMetaType::QStringList, QMetaType::QString,    5,    6,
    QMetaType::Void, QMetaType::QStringList, QMetaType::QString,    5,    6,

       0        // eod
};

void DBusInterface::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DBusInterface *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->ShowFolders((*reinterpret_cast< const QStringList(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->ShowItems((*reinterpret_cast< const QStringList(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 2: _t->ShowItemProperties((*reinterpret_cast< const QStringList(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DBusInterface::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_DBusInterface.data,
    qt_meta_data_DBusInterface,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DBusInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DBusInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DBusInterface.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DBusInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
