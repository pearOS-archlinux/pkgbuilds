/****************************************************************************
** Meta object code from reading C++ file 'folderview_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/folderview_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'folderview_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fm__FolderViewListView_t {
    QByteArrayData data[7];
    char stringdata0[90];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Fm__FolderViewListView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Fm__FolderViewListView_t qt_meta_stringdata_Fm__FolderViewListView = {
    {
QT_MOC_LITERAL(0, 0, 22), // "Fm::FolderViewListView"
QT_MOC_LITERAL(1, 23, 17), // "activatedFiltered"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 11), // "QModelIndex"
QT_MOC_LITERAL(4, 54, 5), // "index"
QT_MOC_LITERAL(5, 60, 18), // "quickLookRequested"
QT_MOC_LITERAL(6, 79, 10) // "activation"

    },
    "Fm::FolderViewListView\0activatedFiltered\0"
    "\0QModelIndex\0index\0quickLookRequested\0"
    "activation"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fm__FolderViewListView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       5,    0,   32,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   33,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void Fm::FolderViewListView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FolderViewListView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->activatedFiltered((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 1: _t->quickLookRequested(); break;
        case 2: _t->activation((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FolderViewListView::*)(const QModelIndex & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderViewListView::activatedFiltered)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FolderViewListView::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderViewListView::quickLookRequested)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Fm::FolderViewListView::staticMetaObject = { {
    QMetaObject::SuperData::link<QListView::staticMetaObject>(),
    qt_meta_stringdata_Fm__FolderViewListView.data,
    qt_meta_data_Fm__FolderViewListView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Fm::FolderViewListView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fm::FolderViewListView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fm__FolderViewListView.stringdata0))
        return static_cast<void*>(this);
    return QListView::qt_metacast(_clname);
}

int Fm::FolderViewListView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListView::qt_metacall(_c, _id, _a);
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
void Fm::FolderViewListView::activatedFiltered(const QModelIndex & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Fm::FolderViewListView::quickLookRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
struct qt_meta_stringdata_Fm__FolderViewTreeView_t {
    QByteArrayData data[9];
    char stringdata0[124];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Fm__FolderViewTreeView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Fm__FolderViewTreeView_t qt_meta_stringdata_Fm__FolderViewTreeView = {
    {
QT_MOC_LITERAL(0, 0, 22), // "Fm::FolderViewTreeView"
QT_MOC_LITERAL(1, 23, 17), // "activatedFiltered"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 11), // "QModelIndex"
QT_MOC_LITERAL(4, 54, 5), // "index"
QT_MOC_LITERAL(5, 60, 18), // "quickLookRequested"
QT_MOC_LITERAL(6, 79, 13), // "layoutColumns"
QT_MOC_LITERAL(7, 93, 10), // "activation"
QT_MOC_LITERAL(8, 104, 19) // "onSortFilterChanged"

    },
    "Fm::FolderViewTreeView\0activatedFiltered\0"
    "\0QModelIndex\0index\0quickLookRequested\0"
    "layoutColumns\0activation\0onSortFilterChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fm__FolderViewTreeView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       5,    0,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   43,    2, 0x08 /* Private */,
       7,    1,   44,    2, 0x08 /* Private */,
       8,    0,   47,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

       0        // eod
};

void Fm::FolderViewTreeView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FolderViewTreeView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->activatedFiltered((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 1: _t->quickLookRequested(); break;
        case 2: _t->layoutColumns(); break;
        case 3: _t->activation((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 4: _t->onSortFilterChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FolderViewTreeView::*)(const QModelIndex & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderViewTreeView::activatedFiltered)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FolderViewTreeView::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderViewTreeView::quickLookRequested)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Fm::FolderViewTreeView::staticMetaObject = { {
    QMetaObject::SuperData::link<QTreeView::staticMetaObject>(),
    qt_meta_stringdata_Fm__FolderViewTreeView.data,
    qt_meta_data_Fm__FolderViewTreeView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Fm::FolderViewTreeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fm::FolderViewTreeView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fm__FolderViewTreeView.stringdata0))
        return static_cast<void*>(this);
    return QTreeView::qt_metacast(_clname);
}

int Fm::FolderViewTreeView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeView::qt_metacall(_c, _id, _a);
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
void Fm::FolderViewTreeView::activatedFiltered(const QModelIndex & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Fm::FolderViewTreeView::quickLookRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
