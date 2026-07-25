/****************************************************************************
** Meta object code from reading C++ file 'windowregistry.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/windowregistry.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'windowregistry.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WindowRegistry_t {
    QByteArrayData data[7];
    char stringdata0[77];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WindowRegistry_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WindowRegistry_t qt_meta_stringdata_WindowRegistry = {
    {
QT_MOC_LITERAL(0, 0, 14), // "WindowRegistry"
QT_MOC_LITERAL(1, 15, 11), // "raiseWindow"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 4), // "path"
QT_MOC_LITERAL(4, 33, 11), // "closeWindow"
QT_MOC_LITERAL(5, 45, 25), // "raiseWindowAndSelectItems"
QT_MOC_LITERAL(6, 71, 5) // "items"

    },
    "WindowRegistry\0raiseWindow\0\0path\0"
    "closeWindow\0raiseWindowAndSelectItems\0"
    "items"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WindowRegistry[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       4,    1,   32,    2, 0x06 /* Public */,
       5,    2,   35,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QStringList,    3,    6,

       0        // eod
};

void WindowRegistry::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WindowRegistry *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->raiseWindow((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->closeWindow((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->raiseWindowAndSelectItems((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (WindowRegistry::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WindowRegistry::raiseWindow)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (WindowRegistry::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WindowRegistry::closeWindow)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (WindowRegistry::*)(const QString & , const QStringList & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WindowRegistry::raiseWindowAndSelectItems)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject WindowRegistry::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_WindowRegistry.data,
    qt_meta_data_WindowRegistry,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WindowRegistry::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WindowRegistry::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WindowRegistry.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WindowRegistry::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void WindowRegistry::raiseWindow(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void WindowRegistry::closeWindow(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void WindowRegistry::raiseWindowAndSelectItems(const QString & _t1, const QStringList & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
