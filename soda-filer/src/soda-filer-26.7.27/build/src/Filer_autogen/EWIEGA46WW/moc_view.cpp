/****************************************************************************
** Meta object code from reading C++ file 'view.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/view.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'view.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Filer__View_t {
    QByteArrayData data[10];
    char stringdata0[97];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Filer__View_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Filer__View_t qt_meta_stringdata_Filer__View = {
    {
QT_MOC_LITERAL(0, 0, 11), // "Filer::View"
QT_MOC_LITERAL(1, 12, 16), // "openDirRequested"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 7), // "FmPath*"
QT_MOC_LITERAL(4, 38, 4), // "path"
QT_MOC_LITERAL(5, 43, 6), // "target"
QT_MOC_LITERAL(6, 50, 11), // "onNewWindow"
QT_MOC_LITERAL(7, 62, 8), // "onNewTab"
QT_MOC_LITERAL(8, 71, 16), // "onOpenInTerminal"
QT_MOC_LITERAL(9, 88, 8) // "onSearch"

    },
    "Filer::View\0openDirRequested\0\0FmPath*\0"
    "path\0target\0onNewWindow\0onNewTab\0"
    "onOpenInTerminal\0onSearch"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Filer__View[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   44,    2, 0x09 /* Protected */,
       7,    0,   45,    2, 0x09 /* Protected */,
       8,    0,   46,    2, 0x09 /* Protected */,
       9,    0,   47,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Filer::View::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<View *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->openDirRequested((*reinterpret_cast< FmPath*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->onNewWindow(); break;
        case 2: _t->onNewTab(); break;
        case 3: _t->onOpenInTerminal(); break;
        case 4: _t->onSearch(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (View::*)(FmPath * , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&View::openDirRequested)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Filer::View::staticMetaObject = { {
    QMetaObject::SuperData::link<Fm::FolderView::staticMetaObject>(),
    qt_meta_stringdata_Filer__View.data,
    qt_meta_data_Filer__View,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Filer::View::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Filer::View::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Filer__View.stringdata0))
        return static_cast<void*>(this);
    return Fm::FolderView::qt_metacast(_clname);
}

int Filer::View::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Fm::FolderView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void Filer::View::openDirRequested(FmPath * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
