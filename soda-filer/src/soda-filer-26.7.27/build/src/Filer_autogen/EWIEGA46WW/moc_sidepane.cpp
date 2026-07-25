/****************************************************************************
** Meta object code from reading C++ file 'sidepane.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/sidepane.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sidepane.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fm__SidePane_t {
    QByteArrayData data[20];
    char stringdata0[322];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Fm__SidePane_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Fm__SidePane_t qt_meta_stringdata_Fm__SidePane = {
    {
QT_MOC_LITERAL(0, 0, 12), // "Fm::SidePane"
QT_MOC_LITERAL(1, 13, 14), // "chdirRequested"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 4), // "type"
QT_MOC_LITERAL(4, 34, 7), // "FmPath*"
QT_MOC_LITERAL(5, 42, 4), // "path"
QT_MOC_LITERAL(6, 47, 30), // "openFolderInNewWindowRequested"
QT_MOC_LITERAL(7, 78, 27), // "openFolderInNewTabRequested"
QT_MOC_LITERAL(8, 106, 29), // "openFolderInTerminalRequested"
QT_MOC_LITERAL(9, 136, 24), // "createNewFolderRequested"
QT_MOC_LITERAL(10, 161, 11), // "modeChanged"
QT_MOC_LITERAL(11, 173, 18), // "Fm::SidePane::Mode"
QT_MOC_LITERAL(12, 192, 4), // "mode"
QT_MOC_LITERAL(13, 197, 15), // "prepareFileMenu"
QT_MOC_LITERAL(14, 213, 13), // "Fm::FileMenu*"
QT_MOC_LITERAL(15, 227, 4), // "menu"
QT_MOC_LITERAL(16, 232, 26), // "onPlacesViewChdirRequested"
QT_MOC_LITERAL(17, 259, 27), // "onDirTreeViewChdirRequested"
QT_MOC_LITERAL(18, 287, 26), // "onComboCurrentIndexChanged"
QT_MOC_LITERAL(19, 314, 7) // "current"

    },
    "Fm::SidePane\0chdirRequested\0\0type\0"
    "FmPath*\0path\0openFolderInNewWindowRequested\0"
    "openFolderInNewTabRequested\0"
    "openFolderInTerminalRequested\0"
    "createNewFolderRequested\0modeChanged\0"
    "Fm::SidePane::Mode\0mode\0prepareFileMenu\0"
    "Fm::FileMenu*\0menu\0onPlacesViewChdirRequested\0"
    "onDirTreeViewChdirRequested\0"
    "onComboCurrentIndexChanged\0current"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fm__SidePane[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   64,    2, 0x06 /* Public */,
       6,    1,   69,    2, 0x06 /* Public */,
       7,    1,   72,    2, 0x06 /* Public */,
       8,    1,   75,    2, 0x06 /* Public */,
       9,    1,   78,    2, 0x06 /* Public */,
      10,    1,   81,    2, 0x06 /* Public */,
      13,    1,   84,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      16,    2,   87,    2, 0x09 /* Protected */,
      17,    2,   92,    2, 0x09 /* Protected */,
      18,    1,   97,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, 0x80000000 | 14,   15,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,
    QMetaType::Void, QMetaType::Int,   19,

       0        // eod
};

void Fm::SidePane::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SidePane *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->chdirRequested((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< FmPath*(*)>(_a[2]))); break;
        case 1: _t->openFolderInNewWindowRequested((*reinterpret_cast< FmPath*(*)>(_a[1]))); break;
        case 2: _t->openFolderInNewTabRequested((*reinterpret_cast< FmPath*(*)>(_a[1]))); break;
        case 3: _t->openFolderInTerminalRequested((*reinterpret_cast< FmPath*(*)>(_a[1]))); break;
        case 4: _t->createNewFolderRequested((*reinterpret_cast< FmPath*(*)>(_a[1]))); break;
        case 5: _t->modeChanged((*reinterpret_cast< Fm::SidePane::Mode(*)>(_a[1]))); break;
        case 6: _t->prepareFileMenu((*reinterpret_cast< Fm::FileMenu*(*)>(_a[1]))); break;
        case 7: _t->onPlacesViewChdirRequested((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< FmPath*(*)>(_a[2]))); break;
        case 8: _t->onDirTreeViewChdirRequested((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< FmPath*(*)>(_a[2]))); break;
        case 9: _t->onComboCurrentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SidePane::*)(int , FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SidePane::chdirRequested)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SidePane::*)(FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SidePane::openFolderInNewWindowRequested)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SidePane::*)(FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SidePane::openFolderInNewTabRequested)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SidePane::*)(FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SidePane::openFolderInTerminalRequested)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (SidePane::*)(FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SidePane::createNewFolderRequested)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (SidePane::*)(Fm::SidePane::Mode );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SidePane::modeChanged)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (SidePane::*)(Fm::FileMenu * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SidePane::prepareFileMenu)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Fm::SidePane::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Fm__SidePane.data,
    qt_meta_data_Fm__SidePane,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Fm::SidePane::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fm::SidePane::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fm__SidePane.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Fm::SidePane::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Fm::SidePane::chdirRequested(int _t1, FmPath * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Fm::SidePane::openFolderInNewWindowRequested(FmPath * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Fm::SidePane::openFolderInNewTabRequested(FmPath * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Fm::SidePane::openFolderInTerminalRequested(FmPath * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Fm::SidePane::createNewFolderRequested(FmPath * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Fm::SidePane::modeChanged(Fm::SidePane::Mode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Fm::SidePane::prepareFileMenu(Fm::FileMenu * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
