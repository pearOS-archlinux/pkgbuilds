/****************************************************************************
** Meta object code from reading C++ file 'smartlauncherbackend.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../plugin/smartlauncherbackend.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'smartlauncherbackend.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN13SmartLauncher7BackendE_t {};
} // unnamed namespace

template <> constexpr inline auto SmartLauncher::Backend::qt_create_metaobjectdata<qt_meta_tag_ZN13SmartLauncher7BackendE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SmartLauncher::Backend",
        "countChanged",
        "",
        "uri",
        "count",
        "countVisibleChanged",
        "countVisible",
        "progressChanged",
        "progress",
        "progressVisibleChanged",
        "progressVisible",
        "urgentChanged",
        "urgent",
        "reloadRequested",
        "launcherRemoved",
        "update",
        "QMap<QString,QVariant>",
        "properties"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'countChanged'
        QtMocHelpers::SignalData<void(const QString &, int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Int, 4 },
        }}),
        // Signal 'countVisibleChanged'
        QtMocHelpers::SignalData<void(const QString &, bool)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Bool, 6 },
        }}),
        // Signal 'progressChanged'
        QtMocHelpers::SignalData<void(const QString &, int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Int, 8 },
        }}),
        // Signal 'progressVisibleChanged'
        QtMocHelpers::SignalData<void(const QString &, bool)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Bool, 10 },
        }}),
        // Signal 'urgentChanged'
        QtMocHelpers::SignalData<void(const QString &, bool)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Bool, 12 },
        }}),
        // Signal 'reloadRequested'
        QtMocHelpers::SignalData<void(const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'launcherRemoved'
        QtMocHelpers::SignalData<void(const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'update'
        QtMocHelpers::SlotData<void(const QString &, const QMap<QString,QVariant> &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { 0x80000000 | 16, 17 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Backend, qt_meta_tag_ZN13SmartLauncher7BackendE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SmartLauncher::Backend::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SmartLauncher7BackendE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SmartLauncher7BackendE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13SmartLauncher7BackendE_t>.metaTypes,
    nullptr
} };

void SmartLauncher::Backend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Backend *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->countChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->countVisibleChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 2: _t->progressChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->progressVisibleChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 4: _t->urgentChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 5: _t->reloadRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->launcherRemoved((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->update((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QMap<QString,QVariant>>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Backend::*)(const QString & , int )>(_a, &Backend::countChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Backend::*)(const QString & , bool )>(_a, &Backend::countVisibleChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Backend::*)(const QString & , int )>(_a, &Backend::progressChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Backend::*)(const QString & , bool )>(_a, &Backend::progressVisibleChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Backend::*)(const QString & , bool )>(_a, &Backend::urgentChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (Backend::*)(const QString & )>(_a, &Backend::reloadRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (Backend::*)(const QString & )>(_a, &Backend::launcherRemoved, 6))
            return;
    }
}

const QMetaObject *SmartLauncher::Backend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SmartLauncher::Backend::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SmartLauncher7BackendE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QDBusContext"))
        return static_cast< QDBusContext*>(this);
    return QObject::qt_metacast(_clname);
}

int SmartLauncher::Backend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void SmartLauncher::Backend::countChanged(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void SmartLauncher::Backend::countVisibleChanged(const QString & _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void SmartLauncher::Backend::progressChanged(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void SmartLauncher::Backend::progressVisibleChanged(const QString & _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void SmartLauncher::Backend::urgentChanged(const QString & _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void SmartLauncher::Backend::reloadRequested(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void SmartLauncher::Backend::launcherRemoved(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}
QT_WARNING_POP
