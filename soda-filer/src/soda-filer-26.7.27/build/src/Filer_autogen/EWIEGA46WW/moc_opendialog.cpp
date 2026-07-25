/****************************************************************************
** Meta object code from reading C++ file 'opendialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/opendialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'opendialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Filer__OpenDialog_t {
    QByteArrayData data[18];
    char stringdata0[227];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Filer__OpenDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Filer__OpenDialog_t qt_meta_stringdata_Filer__OpenDialog = {
    {
QT_MOC_LITERAL(0, 0, 17), // "Filer::OpenDialog"
QT_MOC_LITERAL(1, 18, 11), // "onUpClicked"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 13), // "onOpenClicked"
QT_MOC_LITERAL(4, 45, 13), // "onViewClicked"
QT_MOC_LITERAL(5, 59, 4), // "type"
QT_MOC_LITERAL(6, 64, 11), // "FmFileInfo*"
QT_MOC_LITERAL(7, 76, 4), // "info"
QT_MOC_LITERAL(8, 81, 21), // "onPlaceChdirRequested"
QT_MOC_LITERAL(9, 103, 7), // "FmPath*"
QT_MOC_LITERAL(10, 111, 4), // "path"
QT_MOC_LITERAL(11, 116, 19), // "onColumnDirSelected"
QT_MOC_LITERAL(12, 136, 20), // "onColumnFileSelected"
QT_MOC_LITERAL(13, 157, 8), // "fileInfo"
QT_MOC_LITERAL(14, 166, 21), // "onColumnFileActivated"
QT_MOC_LITERAL(15, 188, 19), // "onSearchTextChanged"
QT_MOC_LITERAL(16, 208, 4), // "text"
QT_MOC_LITERAL(17, 213, 13) // "updateButtons"

    },
    "Filer::OpenDialog\0onUpClicked\0\0"
    "onOpenClicked\0onViewClicked\0type\0"
    "FmFileInfo*\0info\0onPlaceChdirRequested\0"
    "FmPath*\0path\0onColumnDirSelected\0"
    "onColumnFileSelected\0fileInfo\0"
    "onColumnFileActivated\0onSearchTextChanged\0"
    "text\0updateButtons"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Filer__OpenDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x08 /* Private */,
       3,    0,   60,    2, 0x08 /* Private */,
       4,    2,   61,    2, 0x08 /* Private */,
       8,    2,   66,    2, 0x08 /* Private */,
      11,    1,   71,    2, 0x08 /* Private */,
      12,    1,   74,    2, 0x08 /* Private */,
      14,    1,   77,    2, 0x08 /* Private */,
      15,    1,   80,    2, 0x08 /* Private */,
      17,    0,   83,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 6,    5,    7,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 9,    5,   10,
    QMetaType::Void, QMetaType::VoidStar,   10,
    QMetaType::Void, QMetaType::VoidStar,   13,
    QMetaType::Void, QMetaType::VoidStar,   13,
    QMetaType::Void, QMetaType::QString,   16,
    QMetaType::Void,

       0        // eod
};

void Filer::OpenDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<OpenDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onUpClicked(); break;
        case 1: _t->onOpenClicked(); break;
        case 2: _t->onViewClicked((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< FmFileInfo*(*)>(_a[2]))); break;
        case 3: _t->onPlaceChdirRequested((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< FmPath*(*)>(_a[2]))); break;
        case 4: _t->onColumnDirSelected((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 5: _t->onColumnFileSelected((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 6: _t->onColumnFileActivated((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 7: _t->onSearchTextChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->updateButtons(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Filer::OpenDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_Filer__OpenDialog.data,
    qt_meta_data_Filer__OpenDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Filer::OpenDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Filer::OpenDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Filer__OpenDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int Filer::OpenDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
