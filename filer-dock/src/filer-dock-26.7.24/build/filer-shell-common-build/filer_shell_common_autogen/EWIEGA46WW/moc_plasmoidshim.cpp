/****************************************************************************
** Meta object code from reading C++ file 'plasmoidshim.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../filer-shell-common/plasmoidshim.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'plasmoidshim.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14PlasmoidConfigE_t {};
} // unnamed namespace

template <> constexpr inline auto PlasmoidConfig::qt_create_metaobjectdata<qt_meta_tag_ZN14PlasmoidConfigE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PlasmoidConfig",
        "save",
        "",
        "key",
        "QVariant",
        "value",
        "reloadSharedSettings",
        "setStringList"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'save'
        QtMocHelpers::SlotData<void(const QString &, const QVariant &)>(1, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { 0x80000000 | 4, 5 },
        }}),
        // Slot 'reloadSharedSettings'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'setStringList'
        QtMocHelpers::MethodData<void(const QString &, const QStringList &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QStringList, 5 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<PlasmoidConfig, qt_meta_tag_ZN14PlasmoidConfigE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PlasmoidConfig::staticMetaObject = { {
    QMetaObject::SuperData::link<QQmlPropertyMap::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14PlasmoidConfigE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14PlasmoidConfigE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14PlasmoidConfigE_t>.metaTypes,
    nullptr
} };

void PlasmoidConfig::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PlasmoidConfig *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->save((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariant>>(_a[2]))); break;
        case 1: _t->reloadSharedSettings(); break;
        case 2: _t->setStringList((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *PlasmoidConfig::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlasmoidConfig::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14PlasmoidConfigE_t>.strings))
        return static_cast<void*>(this);
    return QQmlPropertyMap::qt_metacast(_clname);
}

int PlasmoidConfig::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQmlPropertyMap::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
namespace {
struct qt_meta_tag_ZN18PlasmoidCoronaShimE_t {};
} // unnamed namespace

template <> constexpr inline auto PlasmoidCoronaShim::qt_create_metaobjectdata<qt_meta_tag_ZN18PlasmoidCoronaShimE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PlasmoidCoronaShim",
        "screenGeometry",
        "QRect",
        "",
        "screenId",
        "availableScreenRect",
        "availableScreenRegion",
        "QRegion",
        "screenForContainment",
        "containment"
    };

    QtMocHelpers::UintData qt_methods {
        // Method 'screenGeometry'
        QtMocHelpers::MethodData<QRect(int) const>(1, 3, QMC::AccessPublic, 0x80000000 | 2, {{
            { QMetaType::Int, 4 },
        }}),
        // Method 'screenGeometry'
        QtMocHelpers::MethodData<QRect() const>(1, 3, QMC::AccessPublic | QMC::MethodCloned, 0x80000000 | 2),
        // Method 'availableScreenRect'
        QtMocHelpers::MethodData<QRect(int) const>(5, 3, QMC::AccessPublic, 0x80000000 | 2, {{
            { QMetaType::Int, 4 },
        }}),
        // Method 'availableScreenRect'
        QtMocHelpers::MethodData<QRect() const>(5, 3, QMC::AccessPublic | QMC::MethodCloned, 0x80000000 | 2),
        // Method 'availableScreenRegion'
        QtMocHelpers::MethodData<QRegion(int) const>(6, 3, QMC::AccessPublic, 0x80000000 | 7, {{
            { QMetaType::Int, 4 },
        }}),
        // Method 'availableScreenRegion'
        QtMocHelpers::MethodData<QRegion() const>(6, 3, QMC::AccessPublic | QMC::MethodCloned, 0x80000000 | 7),
        // Method 'screenForContainment'
        QtMocHelpers::MethodData<int(QObject *) const>(8, 3, QMC::AccessPublic, QMetaType::Int, {{
            { QMetaType::QObjectStar, 9 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<PlasmoidCoronaShim, qt_meta_tag_ZN18PlasmoidCoronaShimE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PlasmoidCoronaShim::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18PlasmoidCoronaShimE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18PlasmoidCoronaShimE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18PlasmoidCoronaShimE_t>.metaTypes,
    nullptr
} };

void PlasmoidCoronaShim::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PlasmoidCoronaShim *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { QRect _r = _t->screenGeometry((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QRect*>(_a[0]) = std::move(_r); }  break;
        case 1: { QRect _r = _t->screenGeometry();
            if (_a[0]) *reinterpret_cast<QRect*>(_a[0]) = std::move(_r); }  break;
        case 2: { QRect _r = _t->availableScreenRect((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QRect*>(_a[0]) = std::move(_r); }  break;
        case 3: { QRect _r = _t->availableScreenRect();
            if (_a[0]) *reinterpret_cast<QRect*>(_a[0]) = std::move(_r); }  break;
        case 4: { QRegion _r = _t->availableScreenRegion((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QRegion*>(_a[0]) = std::move(_r); }  break;
        case 5: { QRegion _r = _t->availableScreenRegion();
            if (_a[0]) *reinterpret_cast<QRegion*>(_a[0]) = std::move(_r); }  break;
        case 6: { int _r = _t->screenForContainment((*reinterpret_cast<std::add_pointer_t<QObject*>>(_a[1])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

const QMetaObject *PlasmoidCoronaShim::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlasmoidCoronaShim::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18PlasmoidCoronaShimE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PlasmoidCoronaShim::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}
namespace {
struct qt_meta_tag_ZN23PlasmoidContainmentShimE_t {};
} // unnamed namespace

template <> constexpr inline auto PlasmoidContainmentShim::qt_create_metaobjectdata<qt_meta_tag_ZN23PlasmoidContainmentShimE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PlasmoidContainmentShim",
        "screenGeometryChanged",
        "",
        "internalAction",
        "QVariant",
        "name",
        "screenGeometry",
        "QRect",
        "corona"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'screenGeometryChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'internalAction'
        QtMocHelpers::MethodData<QVariant(const QString &) const>(3, 2, QMC::AccessPublic, 0x80000000 | 4, {{
            { QMetaType::QString, 5 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'screenGeometry'
        QtMocHelpers::PropertyData<QRect>(6, 0x80000000 | 7, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'corona'
        QtMocHelpers::PropertyData<QObject*>(8, QMetaType::QObjectStar, QMC::DefaultPropertyFlags | QMC::Constant),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<PlasmoidContainmentShim, qt_meta_tag_ZN23PlasmoidContainmentShimE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PlasmoidContainmentShim::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN23PlasmoidContainmentShimE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN23PlasmoidContainmentShimE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN23PlasmoidContainmentShimE_t>.metaTypes,
    nullptr
} };

void PlasmoidContainmentShim::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PlasmoidContainmentShim *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->screenGeometryChanged(); break;
        case 1: { QVariant _r = _t->internalAction((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariant*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (PlasmoidContainmentShim::*)()>(_a, &PlasmoidContainmentShim::screenGeometryChanged, 0))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QRect*>(_v) = _t->screenGeometry(); break;
        case 1: *reinterpret_cast<QObject**>(_v) = _t->corona(); break;
        default: break;
        }
    }
}

const QMetaObject *PlasmoidContainmentShim::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlasmoidContainmentShim::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN23PlasmoidContainmentShimE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PlasmoidContainmentShim::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void PlasmoidContainmentShim::screenGeometryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
namespace {
struct qt_meta_tag_ZN12PlasmoidShimE_t {};
} // unnamed namespace

template <> constexpr inline auto PlasmoidShim::qt_create_metaobjectdata<qt_meta_tag_ZN12PlasmoidShimE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PlasmoidShim",
        "statusChanged",
        "",
        "userConfiguringChanged",
        "backgroundHintsChanged",
        "constraintHintsChanged",
        "contextualActionsAboutToShowSignal",
        "locationChanged",
        "formFactorChanged",
        "iconChanged",
        "titleChanged",
        "expandedChanged",
        "compactRepresentationChanged",
        "fullRepresentationChanged",
        "preferredRepresentationChanged",
        "contextualActionsChanged",
        "modelChanged",
        "globalShortcutChanged",
        "activated",
        "contextualActionsAboutToShow",
        "internalAction",
        "QVariant",
        "name",
        "configuration",
        "PlasmoidConfig*",
        "containment",
        "PlasmoidContainmentShim*",
        "location",
        "formFactor",
        "immutability",
        "pluginName",
        "status",
        "userConfiguring",
        "backgroundHints",
        "constraintHints",
        "containmentDisplayHints",
        "icon",
        "title",
        "expanded",
        "compactRepresentation",
        "fullRepresentation",
        "preferredRepresentation",
        "metaData",
        "QVariantMap",
        "contextualActions",
        "QQmlListProperty<QObject>",
        "id",
        "model",
        "globalShortcut",
        "NoHint",
        "CanFillArea"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'statusChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'userConfiguringChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'backgroundHintsChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'constraintHintsChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'contextualActionsAboutToShowSignal'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'locationChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'formFactorChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'iconChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'titleChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'expandedChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'compactRepresentationChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fullRepresentationChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'preferredRepresentationChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'contextualActionsChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'modelChanged'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'globalShortcutChanged'
        QtMocHelpers::SignalData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'activated'
        QtMocHelpers::SignalData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'contextualActionsAboutToShow'
        QtMocHelpers::MethodData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'internalAction'
        QtMocHelpers::MethodData<QVariant(const QString &) const>(20, 2, QMC::AccessPublic, 0x80000000 | 21, {{
            { QMetaType::QString, 22 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'configuration'
        QtMocHelpers::PropertyData<PlasmoidConfig*>(23, 0x80000000 | 24, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'containment'
        QtMocHelpers::PropertyData<PlasmoidContainmentShim*>(25, 0x80000000 | 26, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'location'
        QtMocHelpers::PropertyData<int>(27, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'formFactor'
        QtMocHelpers::PropertyData<int>(28, QMetaType::Int, QMC::DefaultPropertyFlags, 6),
        // property 'immutability'
        QtMocHelpers::PropertyData<int>(29, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'pluginName'
        QtMocHelpers::PropertyData<QString>(30, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'status'
        QtMocHelpers::PropertyData<int>(31, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'userConfiguring'
        QtMocHelpers::PropertyData<bool>(32, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'backgroundHints'
        QtMocHelpers::PropertyData<int>(33, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'constraintHints'
        QtMocHelpers::PropertyData<int>(34, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'containmentDisplayHints'
        QtMocHelpers::PropertyData<int>(35, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'icon'
        QtMocHelpers::PropertyData<QVariant>(36, 0x80000000 | 21, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 7),
        // property 'title'
        QtMocHelpers::PropertyData<QString>(37, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 8),
        // property 'expanded'
        QtMocHelpers::PropertyData<bool>(38, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 9),
        // property 'compactRepresentation'
        QtMocHelpers::PropertyData<QVariant>(39, 0x80000000 | 21, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 10),
        // property 'fullRepresentation'
        QtMocHelpers::PropertyData<QVariant>(40, 0x80000000 | 21, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 11),
        // property 'preferredRepresentation'
        QtMocHelpers::PropertyData<QVariant>(41, 0x80000000 | 21, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 12),
        // property 'metaData'
        QtMocHelpers::PropertyData<QVariantMap>(42, 0x80000000 | 43, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'contextualActions'
        QtMocHelpers::PropertyData<QQmlListProperty<QObject>>(44, 0x80000000 | 45, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 13),
        // property 'id'
        QtMocHelpers::PropertyData<QString>(46, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'model'
        QtMocHelpers::PropertyData<QVariant>(47, 0x80000000 | 21, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 14),
        // property 'globalShortcut'
        QtMocHelpers::PropertyData<QString>(48, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 15),
        // property 'NoHint'
        QtMocHelpers::PropertyData<int>(49, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'CanFillArea'
        QtMocHelpers::PropertyData<int>(50, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<PlasmoidShim, qt_meta_tag_ZN12PlasmoidShimE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PlasmoidShim::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PlasmoidShimE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PlasmoidShimE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12PlasmoidShimE_t>.metaTypes,
    nullptr
} };

void PlasmoidShim::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PlasmoidShim *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->statusChanged(); break;
        case 1: _t->userConfiguringChanged(); break;
        case 2: _t->backgroundHintsChanged(); break;
        case 3: _t->constraintHintsChanged(); break;
        case 4: _t->contextualActionsAboutToShowSignal(); break;
        case 5: _t->locationChanged(); break;
        case 6: _t->formFactorChanged(); break;
        case 7: _t->iconChanged(); break;
        case 8: _t->titleChanged(); break;
        case 9: _t->expandedChanged(); break;
        case 10: _t->compactRepresentationChanged(); break;
        case 11: _t->fullRepresentationChanged(); break;
        case 12: _t->preferredRepresentationChanged(); break;
        case 13: _t->contextualActionsChanged(); break;
        case 14: _t->modelChanged(); break;
        case 15: _t->globalShortcutChanged(); break;
        case 16: _t->activated(); break;
        case 17: _t->contextualActionsAboutToShow(); break;
        case 18: { QVariant _r = _t->internalAction((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariant*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::statusChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::userConfiguringChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::backgroundHintsChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::constraintHintsChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::contextualActionsAboutToShowSignal, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::locationChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::formFactorChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::iconChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::titleChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::expandedChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::compactRepresentationChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::fullRepresentationChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::preferredRepresentationChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::contextualActionsChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::modelChanged, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::globalShortcutChanged, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (PlasmoidShim::*)()>(_a, &PlasmoidShim::activated, 16))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< PlasmoidConfig* >(); break;
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< PlasmoidContainmentShim* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<PlasmoidConfig**>(_v) = _t->configuration(); break;
        case 1: *reinterpret_cast<PlasmoidContainmentShim**>(_v) = _t->containment(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->location(); break;
        case 3: *reinterpret_cast<int*>(_v) = _t->formFactor(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->immutability(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->pluginName(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->status(); break;
        case 7: *reinterpret_cast<bool*>(_v) = _t->userConfiguring(); break;
        case 8: *reinterpret_cast<int*>(_v) = _t->backgroundHints(); break;
        case 9: *reinterpret_cast<int*>(_v) = _t->constraintHints(); break;
        case 10: *reinterpret_cast<int*>(_v) = _t->containmentDisplayHints(); break;
        case 11: *reinterpret_cast<QVariant*>(_v) = _t->icon(); break;
        case 12: *reinterpret_cast<QString*>(_v) = _t->title(); break;
        case 13: *reinterpret_cast<bool*>(_v) = _t->expanded(); break;
        case 14: *reinterpret_cast<QVariant*>(_v) = _t->compactRepresentation(); break;
        case 15: *reinterpret_cast<QVariant*>(_v) = _t->fullRepresentation(); break;
        case 16: *reinterpret_cast<QVariant*>(_v) = _t->preferredRepresentation(); break;
        case 17: *reinterpret_cast<QVariantMap*>(_v) = _t->metaData(); break;
        case 18: *reinterpret_cast<QQmlListProperty<QObject>*>(_v) = _t->contextualActions(); break;
        case 19: *reinterpret_cast<QString*>(_v) = _t->id(); break;
        case 20: *reinterpret_cast<QVariant*>(_v) = _t->model(); break;
        case 21: *reinterpret_cast<QString*>(_v) = _t->globalShortcut(); break;
        case 22: *reinterpret_cast<int*>(_v) = _t->noHint(); break;
        case 23: *reinterpret_cast<int*>(_v) = _t->canFillArea(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 6: _t->setStatus(*reinterpret_cast<int*>(_v)); break;
        case 7: _t->setUserConfiguring(*reinterpret_cast<bool*>(_v)); break;
        case 8: _t->setBackgroundHints(*reinterpret_cast<int*>(_v)); break;
        case 9: _t->setConstraintHints(*reinterpret_cast<int*>(_v)); break;
        case 11: _t->setIcon(*reinterpret_cast<QVariant*>(_v)); break;
        case 12: _t->setTitle(*reinterpret_cast<QString*>(_v)); break;
        case 13: _t->setExpanded(*reinterpret_cast<bool*>(_v)); break;
        case 14: _t->setCompactRepresentation(*reinterpret_cast<QVariant*>(_v)); break;
        case 15: _t->setFullRepresentation(*reinterpret_cast<QVariant*>(_v)); break;
        case 16: _t->setPreferredRepresentation(*reinterpret_cast<QVariant*>(_v)); break;
        case 20: _t->setModel(*reinterpret_cast<QVariant*>(_v)); break;
        case 21: _t->setGlobalShortcut(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *PlasmoidShim::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlasmoidShim::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PlasmoidShimE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PlasmoidShim::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 19;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    return _id;
}

// SIGNAL 0
void PlasmoidShim::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PlasmoidShim::userConfiguringChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PlasmoidShim::backgroundHintsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PlasmoidShim::constraintHintsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PlasmoidShim::contextualActionsAboutToShowSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PlasmoidShim::locationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void PlasmoidShim::formFactorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void PlasmoidShim::iconChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void PlasmoidShim::titleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void PlasmoidShim::expandedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void PlasmoidShim::compactRepresentationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void PlasmoidShim::fullRepresentationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void PlasmoidShim::preferredRepresentationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void PlasmoidShim::contextualActionsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void PlasmoidShim::modelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void PlasmoidShim::globalShortcutChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}

// SIGNAL 16
void PlasmoidShim::activated()
{
    QMetaObject::activate(this, &staticMetaObject, 16, nullptr);
}
QT_WARNING_POP
