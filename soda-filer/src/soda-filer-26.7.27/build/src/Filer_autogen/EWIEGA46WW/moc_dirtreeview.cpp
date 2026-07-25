/****************************************************************************
** Meta object code from reading C++ file 'dirtreeview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/dirtreeview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dirtreeview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fm__DirTreeView_t {
    QByteArrayData data[29];
    char stringdata0[396];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Fm__DirTreeView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Fm__DirTreeView_t qt_meta_stringdata_Fm__DirTreeView = {
    {
QT_MOC_LITERAL(0, 0, 15), // "Fm::DirTreeView"
QT_MOC_LITERAL(1, 16, 14), // "chdirRequested"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 4), // "type"
QT_MOC_LITERAL(4, 37, 7), // "FmPath*"
QT_MOC_LITERAL(5, 45, 4), // "path"
QT_MOC_LITERAL(6, 50, 30), // "openFolderInNewWindowRequested"
QT_MOC_LITERAL(7, 81, 27), // "openFolderInNewTabRequested"
QT_MOC_LITERAL(8, 109, 29), // "openFolderInTerminalRequested"
QT_MOC_LITERAL(9, 139, 24), // "createNewFolderRequested"
QT_MOC_LITERAL(10, 164, 15), // "prepareFileMenu"
QT_MOC_LITERAL(11, 180, 13), // "Fm::FileMenu*"
QT_MOC_LITERAL(12, 194, 4), // "menu"
QT_MOC_LITERAL(13, 199, 11), // "onCollapsed"
QT_MOC_LITERAL(14, 211, 11), // "QModelIndex"
QT_MOC_LITERAL(15, 223, 5), // "index"
QT_MOC_LITERAL(16, 229, 10), // "onExpanded"
QT_MOC_LITERAL(17, 240, 11), // "onRowLoaded"
QT_MOC_LITERAL(18, 252, 18), // "onSelectionChanged"
QT_MOC_LITERAL(19, 271, 14), // "QItemSelection"
QT_MOC_LITERAL(20, 286, 8), // "selected"
QT_MOC_LITERAL(21, 295, 10), // "deselected"
QT_MOC_LITERAL(22, 306, 28), // "onCustomContextMenuRequested"
QT_MOC_LITERAL(23, 335, 3), // "pos"
QT_MOC_LITERAL(24, 339, 6), // "onOpen"
QT_MOC_LITERAL(25, 346, 11), // "onNewWindow"
QT_MOC_LITERAL(26, 358, 8), // "onNewTab"
QT_MOC_LITERAL(27, 367, 16), // "onOpenInTerminal"
QT_MOC_LITERAL(28, 384, 11) // "onNewFolder"

    },
    "Fm::DirTreeView\0chdirRequested\0\0type\0"
    "FmPath*\0path\0openFolderInNewWindowRequested\0"
    "openFolderInNewTabRequested\0"
    "openFolderInTerminalRequested\0"
    "createNewFolderRequested\0prepareFileMenu\0"
    "Fm::FileMenu*\0menu\0onCollapsed\0"
    "QModelIndex\0index\0onExpanded\0onRowLoaded\0"
    "onSelectionChanged\0QItemSelection\0"
    "selected\0deselected\0onCustomContextMenuRequested\0"
    "pos\0onOpen\0onNewWindow\0onNewTab\0"
    "onOpenInTerminal\0onNewFolder"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fm__DirTreeView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   94,    2, 0x06 /* Public */,
       6,    1,   99,    2, 0x06 /* Public */,
       7,    1,  102,    2, 0x06 /* Public */,
       8,    1,  105,    2, 0x06 /* Public */,
       9,    1,  108,    2, 0x06 /* Public */,
      10,    1,  111,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    1,  114,    2, 0x09 /* Protected */,
      16,    1,  117,    2, 0x09 /* Protected */,
      17,    1,  120,    2, 0x09 /* Protected */,
      18,    2,  123,    2, 0x09 /* Protected */,
      22,    1,  128,    2, 0x09 /* Protected */,
      24,    0,  131,    2, 0x09 /* Protected */,
      25,    0,  132,    2, 0x09 /* Protected */,
      26,    0,  133,    2, 0x09 /* Protected */,
      27,    0,  134,    2, 0x09 /* Protected */,
      28,    0,  135,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 11,   12,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 19, 0x80000000 | 19,   20,   21,
    QMetaType::Void, QMetaType::QPoint,   23,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Fm::DirTreeView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DirTreeView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->chdirRequested((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< FmPath*(*)>(_a[2]))); break;
        case 1: _t->openFolderInNewWindowRequested((*reinterpret_cast< FmPath*(*)>(_a[1]))); break;
        case 2: _t->openFolderInNewTabRequested((*reinterpret_cast< FmPath*(*)>(_a[1]))); break;
        case 3: _t->openFolderInTerminalRequested((*reinterpret_cast< FmPath*(*)>(_a[1]))); break;
        case 4: _t->createNewFolderRequested((*reinterpret_cast< FmPath*(*)>(_a[1]))); break;
        case 5: _t->prepareFileMenu((*reinterpret_cast< Fm::FileMenu*(*)>(_a[1]))); break;
        case 6: _t->onCollapsed((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 7: _t->onExpanded((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 8: _t->onRowLoaded((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 9: _t->onSelectionChanged((*reinterpret_cast< const QItemSelection(*)>(_a[1])),(*reinterpret_cast< const QItemSelection(*)>(_a[2]))); break;
        case 10: _t->onCustomContextMenuRequested((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 11: _t->onOpen(); break;
        case 12: _t->onNewWindow(); break;
        case 13: _t->onNewTab(); break;
        case 14: _t->onOpenInTerminal(); break;
        case 15: _t->onNewFolder(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 9:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QItemSelection >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DirTreeView::*)(int , FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DirTreeView::chdirRequested)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DirTreeView::*)(FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DirTreeView::openFolderInNewWindowRequested)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DirTreeView::*)(FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DirTreeView::openFolderInNewTabRequested)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DirTreeView::*)(FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DirTreeView::openFolderInTerminalRequested)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DirTreeView::*)(FmPath * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DirTreeView::createNewFolderRequested)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DirTreeView::*)(Fm::FileMenu * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DirTreeView::prepareFileMenu)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Fm::DirTreeView::staticMetaObject = { {
    QMetaObject::SuperData::link<QTreeView::staticMetaObject>(),
    qt_meta_stringdata_Fm__DirTreeView.data,
    qt_meta_data_Fm__DirTreeView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Fm::DirTreeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fm::DirTreeView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fm__DirTreeView.stringdata0))
        return static_cast<void*>(this);
    return QTreeView::qt_metacast(_clname);
}

int Fm::DirTreeView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void Fm::DirTreeView::chdirRequested(int _t1, FmPath * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Fm::DirTreeView::openFolderInNewWindowRequested(FmPath * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Fm::DirTreeView::openFolderInNewTabRequested(FmPath * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Fm::DirTreeView::openFolderInTerminalRequested(FmPath * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Fm::DirTreeView::createNewFolderRequested(FmPath * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Fm::DirTreeView::prepareFileMenu(Fm::FileMenu * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
