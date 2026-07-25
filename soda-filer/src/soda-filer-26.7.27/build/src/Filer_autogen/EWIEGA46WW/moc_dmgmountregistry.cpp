/****************************************************************************
** Meta object code from reading C++ file 'dmgmountregistry.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/dmgmountregistry.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dmgmountregistry.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Filer__DmgMountRegistry_t {
    QByteArrayData data[7];
    char stringdata0[76];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Filer__DmgMountRegistry_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Filer__DmgMountRegistry_t qt_meta_stringdata_Filer__DmgMountRegistry = {
    {
QT_MOC_LITERAL(0, 0, 23), // "Filer::DmgMountRegistry"
QT_MOC_LITERAL(1, 24, 10), // "mountAdded"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 15), // "Filer::DmgMount"
QT_MOC_LITERAL(4, 52, 5), // "mount"
QT_MOC_LITERAL(5, 58, 12), // "mountRemoved"
QT_MOC_LITERAL(6, 71, 4) // "path"

    },
    "Filer::DmgMountRegistry\0mountAdded\0\0"
    "Filer::DmgMount\0mount\0mountRemoved\0"
    "path"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Filer__DmgMountRegistry[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       5,    1,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,

       0        // eod
};

void Filer::DmgMountRegistry::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DmgMountRegistry *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->mountAdded((*reinterpret_cast< const Filer::DmgMount(*)>(_a[1]))); break;
        case 1: _t->mountRemoved((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DmgMountRegistry::*)(const Filer::DmgMount & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DmgMountRegistry::mountAdded)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DmgMountRegistry::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DmgMountRegistry::mountRemoved)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Filer::DmgMountRegistry::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Filer__DmgMountRegistry.data,
    qt_meta_data_Filer__DmgMountRegistry,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Filer::DmgMountRegistry::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Filer::DmgMountRegistry::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Filer__DmgMountRegistry.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Filer::DmgMountRegistry::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Filer::DmgMountRegistry::mountAdded(const Filer::DmgMount & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Filer::DmgMountRegistry::mountRemoved(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
