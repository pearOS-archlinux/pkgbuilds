/****************************************************************************
** Meta object code from reading C++ file 'application.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/application.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'application.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Filer__Application_t {
    QByteArrayData data[29];
    char stringdata0[447];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Filer__Application_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Filer__Application_t qt_meta_stringdata_Filer__Application = {
    {
QT_MOC_LITERAL(0, 0, 18), // "Filer::Application"
QT_MOC_LITERAL(1, 19, 24), // "openFolderAndSelectItems"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 6), // "folder"
QT_MOC_LITERAL(4, 52, 5), // "items"
QT_MOC_LITERAL(5, 58, 13), // "onAboutToQuit"
QT_MOC_LITERAL(6, 72, 17), // "onSigtermNotified"
QT_MOC_LITERAL(7, 90, 18), // "onLastWindowClosed"
QT_MOC_LITERAL(8, 109, 18), // "onSaveStateRequest"
QT_MOC_LITERAL(9, 128, 16), // "QSessionManager&"
QT_MOC_LITERAL(10, 145, 7), // "manager"
QT_MOC_LITERAL(11, 153, 15), // "onScreenResized"
QT_MOC_LITERAL(12, 169, 3), // "num"
QT_MOC_LITERAL(13, 173, 20), // "onScreenCountChanged"
QT_MOC_LITERAL(14, 194, 8), // "newCount"
QT_MOC_LITERAL(15, 203, 17), // "initVolumeManager"
QT_MOC_LITERAL(16, 221, 24), // "onVirtualGeometryChanged"
QT_MOC_LITERAL(17, 246, 4), // "rect"
QT_MOC_LITERAL(18, 251, 17), // "onScreenDestroyed"
QT_MOC_LITERAL(19, 269, 9), // "screenObj"
QT_MOC_LITERAL(20, 279, 13), // "onScreenAdded"
QT_MOC_LITERAL(21, 293, 8), // "QScreen*"
QT_MOC_LITERAL(22, 302, 9), // "newScreen"
QT_MOC_LITERAL(23, 312, 22), // "reloadDesktopsAsNeeded"
QT_MOC_LITERAL(24, 335, 18), // "onFindFileAccepted"
QT_MOC_LITERAL(25, 354, 17), // "onUserDirsChanged"
QT_MOC_LITERAL(26, 372, 26), // "onSystemThemeConfigChanged"
QT_MOC_LITERAL(27, 399, 25), // "onTintSettingsFileChanged"
QT_MOC_LITERAL(28, 425, 21) // "desktopManagerEnabled"

    },
    "Filer::Application\0openFolderAndSelectItems\0"
    "\0folder\0items\0onAboutToQuit\0"
    "onSigtermNotified\0onLastWindowClosed\0"
    "onSaveStateRequest\0QSessionManager&\0"
    "manager\0onScreenResized\0num\0"
    "onScreenCountChanged\0newCount\0"
    "initVolumeManager\0onVirtualGeometryChanged\0"
    "rect\0onScreenDestroyed\0screenObj\0"
    "onScreenAdded\0QScreen*\0newScreen\0"
    "reloadDesktopsAsNeeded\0onFindFileAccepted\0"
    "onUserDirsChanged\0onSystemThemeConfigChanged\0"
    "onTintSettingsFileChanged\0"
    "desktopManagerEnabled"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Filer__Application[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       1,  126, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   94,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   99,    2, 0x09 /* Protected */,
       6,    0,  100,    2, 0x09 /* Protected */,
       7,    0,  101,    2, 0x09 /* Protected */,
       8,    1,  102,    2, 0x09 /* Protected */,
      11,    1,  105,    2, 0x09 /* Protected */,
      13,    1,  108,    2, 0x09 /* Protected */,
      15,    0,  111,    2, 0x09 /* Protected */,
      16,    1,  112,    2, 0x09 /* Protected */,
      18,    1,  115,    2, 0x09 /* Protected */,
      20,    1,  118,    2, 0x09 /* Protected */,
      23,    0,  121,    2, 0x09 /* Protected */,
      24,    0,  122,    2, 0x09 /* Protected */,
      25,    0,  123,    2, 0x08 /* Private */,
      26,    0,  124,    2, 0x08 /* Private */,
      27,    0,  125,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QStringList,    3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QRect,   17,
    QMetaType::Void, QMetaType::QObjectStar,   19,
    QMetaType::Void, 0x80000000 | 21,   22,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      28, QMetaType::Bool, 0x00095001,

       0        // eod
};

void Filer::Application::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Application *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->openFolderAndSelectItems((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QStringList(*)>(_a[2]))); break;
        case 1: _t->onAboutToQuit(); break;
        case 2: _t->onSigtermNotified(); break;
        case 3: _t->onLastWindowClosed(); break;
        case 4: _t->onSaveStateRequest((*reinterpret_cast< QSessionManager(*)>(_a[1]))); break;
        case 5: _t->onScreenResized((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->onScreenCountChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->initVolumeManager(); break;
        case 8: _t->onVirtualGeometryChanged((*reinterpret_cast< const QRect(*)>(_a[1]))); break;
        case 9: _t->onScreenDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        case 10: _t->onScreenAdded((*reinterpret_cast< QScreen*(*)>(_a[1]))); break;
        case 11: _t->reloadDesktopsAsNeeded(); break;
        case 12: _t->onFindFileAccepted(); break;
        case 13: _t->onUserDirsChanged(); break;
        case 14: _t->onSystemThemeConfigChanged(); break;
        case 15: _t->onTintSettingsFileChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Application::*)(QString , QStringList );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Application::openFolderAndSelectItems)) {
                *result = 0;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<Application *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->desktopManagerEnabled(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject Filer::Application::staticMetaObject = { {
    QMetaObject::SuperData::link<QApplication::staticMetaObject>(),
    qt_meta_stringdata_Filer__Application.data,
    qt_meta_data_Filer__Application,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Filer::Application::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Filer::Application::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Filer__Application.stringdata0))
        return static_cast<void*>(this);
    return QApplication::qt_metacast(_clname);
}

int Filer::Application::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QApplication::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void Filer::Application::openFolderAndSelectItems(QString _t1, QStringList _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
