/****************************************************************************
** Meta object code from reading C++ file 'windowgeometrytracker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/windowgeometrytracker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'windowgeometrytracker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WindowGeometryTracker_t {
    QByteArrayData data[11];
    char stringdata0[104];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WindowGeometryTracker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WindowGeometryTracker_t qt_meta_stringdata_WindowGeometryTracker = {
    {
QT_MOC_LITERAL(0, 0, 21), // "WindowGeometryTracker"
QT_MOC_LITERAL(1, 22, 15), // "D-Bus Interface"
QT_MOC_LITERAL(2, 38, 23), // "org.filer.WindowTracker"
QT_MOC_LITERAL(3, 62, 20), // "reportWindowGeometry"
QT_MOC_LITERAL(4, 83, 0), // ""
QT_MOC_LITERAL(5, 84, 3), // "pid"
QT_MOC_LITERAL(6, 88, 7), // "caption"
QT_MOC_LITERAL(7, 96, 1), // "x"
QT_MOC_LITERAL(8, 98, 1), // "y"
QT_MOC_LITERAL(9, 100, 1), // "w"
QT_MOC_LITERAL(10, 102, 1) // "h"

    },
    "WindowGeometryTracker\0D-Bus Interface\0"
    "org.filer.WindowTracker\0reportWindowGeometry\0"
    "\0pid\0caption\0x\0y\0w\0h"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WindowGeometryTracker[] = {

 // content:
       8,       // revision
       0,       // classname
       1,   14, // classinfo
       1,   16, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // classinfo: key, value
       1,    2,

 // methods: name, argc, parameters, tag, flags
       3,    6,   21,    4, 0x42 /* Public | isScriptable */,

 // methods: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::QString, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,    5,    6,    7,    8,    9,   10,

       0        // eod
};

void WindowGeometryTracker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WindowGeometryTracker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->reportWindowGeometry((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])),(*reinterpret_cast< int(*)>(_a[6]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject WindowGeometryTracker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_WindowGeometryTracker.data,
    qt_meta_data_WindowGeometryTracker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WindowGeometryTracker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WindowGeometryTracker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WindowGeometryTracker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WindowGeometryTracker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
