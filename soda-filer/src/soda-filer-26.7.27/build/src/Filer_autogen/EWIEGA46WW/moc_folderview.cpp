/****************************************************************************
** Meta object code from reading C++ file 'folderview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/folderview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'folderview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fm__FolderView_t {
    QByteArrayData data[25];
    char stringdata0[304];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Fm__FolderView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Fm__FolderView_t qt_meta_stringdata_Fm__FolderView = {
    {
QT_MOC_LITERAL(0, 0, 14), // "Fm::FolderView"
QT_MOC_LITERAL(1, 15, 7), // "clicked"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 4), // "type"
QT_MOC_LITERAL(4, 29, 11), // "FmFileInfo*"
QT_MOC_LITERAL(5, 41, 4), // "file"
QT_MOC_LITERAL(6, 46, 11), // "clickedBack"
QT_MOC_LITERAL(7, 58, 14), // "clickedForward"
QT_MOC_LITERAL(8, 73, 10), // "selChanged"
QT_MOC_LITERAL(9, 84, 5), // "n_sel"
QT_MOC_LITERAL(10, 90, 11), // "sortChanged"
QT_MOC_LITERAL(11, 102, 18), // "quickLookRequested"
QT_MOC_LITERAL(12, 121, 15), // "onItemActivated"
QT_MOC_LITERAL(13, 137, 11), // "QModelIndex"
QT_MOC_LITERAL(14, 149, 5), // "index"
QT_MOC_LITERAL(15, 155, 18), // "onSelectionChanged"
QT_MOC_LITERAL(16, 174, 14), // "QItemSelection"
QT_MOC_LITERAL(17, 189, 8), // "selected"
QT_MOC_LITERAL(18, 198, 10), // "deselected"
QT_MOC_LITERAL(19, 209, 13), // "onFileClicked"
QT_MOC_LITERAL(20, 223, 8), // "fileInfo"
QT_MOC_LITERAL(21, 232, 23), // "selectFileAfterCreation"
QT_MOC_LITERAL(22, 256, 4), // "path"
QT_MOC_LITERAL(23, 261, 22), // "onAutoSelectionTimeout"
QT_MOC_LITERAL(24, 284, 19) // "onSelChangedTimeout"

    },
    "Fm::FolderView\0clicked\0\0type\0FmFileInfo*\0"
    "file\0clickedBack\0clickedForward\0"
    "selChanged\0n_sel\0sortChanged\0"
    "quickLookRequested\0onItemActivated\0"
    "QModelIndex\0index\0onSelectionChanged\0"
    "QItemSelection\0selected\0deselected\0"
    "onFileClicked\0fileInfo\0selectFileAfterCreation\0"
    "path\0onAutoSelectionTimeout\0"
    "onSelChangedTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fm__FolderView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   74,    2, 0x06 /* Public */,
       6,    0,   79,    2, 0x06 /* Public */,
       7,    0,   80,    2, 0x06 /* Public */,
       8,    1,   81,    2, 0x06 /* Public */,
      10,    0,   84,    2, 0x06 /* Public */,
      11,    0,   85,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    1,   86,    2, 0x0a /* Public */,
      15,    2,   89,    2, 0x0a /* Public */,
      19,    2,   94,    2, 0x0a /* Public */,
      21,    1,   99,    2, 0x0a /* Public */,
      23,    0,  102,    2, 0x08 /* Private */,
      24,    0,  103,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void, 0x80000000 | 16, 0x80000000 | 16,   17,   18,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,   20,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Fm::FolderView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FolderView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->clicked((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< FmFileInfo*(*)>(_a[2]))); break;
        case 1: _t->clickedBack(); break;
        case 2: _t->clickedForward(); break;
        case 3: _t->selChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->sortChanged(); break;
        case 5: _t->quickLookRequested(); break;
        case 6: _t->onItemActivated((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 7: _t->onSelectionChanged((*reinterpret_cast< const QItemSelection(*)>(_a[1])),(*reinterpret_cast< const QItemSelection(*)>(_a[2]))); break;
        case 8: _t->onFileClicked((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< FmFileInfo*(*)>(_a[2]))); break;
        case 9: _t->selectFileAfterCreation((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 10: _t->onAutoSelectionTimeout(); break;
        case 11: _t->onSelChangedTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 7:
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
            using _t = void (FolderView::*)(int , FmFileInfo * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderView::clicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FolderView::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderView::clickedBack)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (FolderView::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderView::clickedForward)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (FolderView::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderView::selChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (FolderView::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderView::sortChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (FolderView::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FolderView::quickLookRequested)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Fm::FolderView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Fm__FolderView.data,
    qt_meta_data_Fm__FolderView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Fm::FolderView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fm::FolderView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fm__FolderView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Fm::FolderView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void Fm::FolderView::clicked(int _t1, FmFileInfo * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Fm::FolderView::clickedBack()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Fm::FolderView::clickedForward()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Fm::FolderView::selChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Fm::FolderView::sortChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void Fm::FolderView::quickLookRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
