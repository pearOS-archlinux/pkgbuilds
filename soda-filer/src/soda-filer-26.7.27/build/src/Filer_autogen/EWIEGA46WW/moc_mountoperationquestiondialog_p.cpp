/****************************************************************************
** Meta object code from reading C++ file 'mountoperationquestiondialog_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/mountoperationquestiondialog_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mountoperationquestiondialog_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fm__MountOperationQuestionDialog_t {
    QByteArrayData data[5];
    char stringdata0[74];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Fm__MountOperationQuestionDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Fm__MountOperationQuestionDialog_t qt_meta_stringdata_Fm__MountOperationQuestionDialog = {
    {
QT_MOC_LITERAL(0, 0, 32), // "Fm::MountOperationQuestionDialog"
QT_MOC_LITERAL(1, 33, 15), // "onButtonClicked"
QT_MOC_LITERAL(2, 49, 0), // ""
QT_MOC_LITERAL(3, 50, 16), // "QAbstractButton*"
QT_MOC_LITERAL(4, 67, 6) // "button"

    },
    "Fm::MountOperationQuestionDialog\0"
    "onButtonClicked\0\0QAbstractButton*\0"
    "button"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fm__MountOperationQuestionDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void Fm::MountOperationQuestionDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MountOperationQuestionDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onButtonClicked((*reinterpret_cast< QAbstractButton*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Fm::MountOperationQuestionDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QMessageBox::staticMetaObject>(),
    qt_meta_stringdata_Fm__MountOperationQuestionDialog.data,
    qt_meta_data_Fm__MountOperationQuestionDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Fm::MountOperationQuestionDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fm::MountOperationQuestionDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fm__MountOperationQuestionDialog.stringdata0))
        return static_cast<void*>(this);
    return QMessageBox::qt_metacast(_clname);
}

int Fm::MountOperationQuestionDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMessageBox::qt_metacall(_c, _id, _a);
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
