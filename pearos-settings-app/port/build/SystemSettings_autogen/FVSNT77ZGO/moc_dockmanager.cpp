/****************************************************************************
** Meta object code from reading C++ file 'dockmanager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../backend/dockmanager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dockmanager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11DockManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto DockManager::qt_create_metaobjectdata<qt_meta_tag_ZN11DockManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DockManager",
        "changed",
        "",
        "refresh",
        "set",
        "key",
        "QVariant",
        "value",
        "skinName",
        "iconSize",
        "magnification",
        "amplitud",
        "iconSpacing",
        "showReflection",
        "skinBlur",
        "skinBlurRadius",
        "skinLiquidGelEffect",
        "skinRefractionStrength",
        "skinRgbFringing",
        "skinPositionTaskIndicator",
        "showToolTips",
        "highlightWindows",
        "indicateAudioStreams",
        "interactiveMute",
        "tooltipControls",
        "minimizeActiveTaskOnClick",
        "middleClickAction",
        "wheelEnabled",
        "wheelSkipMinimized",
        "groupingStrategy",
        "groupedTaskVisualization",
        "sortingStrategy",
        "showOnlyCurrentDesktop",
        "showOnlyCurrentActivity",
        "showOnlyCurrentScreen",
        "showOnlyMinimized",
        "unhideOnAttention",
        "skinLeftMargin",
        "skinTopMargin",
        "skinRightMargin",
        "skinBottomMargin",
        "skinOutsideLeftMargin",
        "skinOutsideTopMargin",
        "skinOutsideRightMargin",
        "skinOutsideBottomMargin",
        "availableSkins"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'changed'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refresh'
        QtMocHelpers::MethodData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'set'
        QtMocHelpers::MethodData<void(const QString &, const QVariant &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { 0x80000000 | 6, 7 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'skinName'
        QtMocHelpers::PropertyData<QString>(8, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'iconSize'
        QtMocHelpers::PropertyData<int>(9, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'magnification'
        QtMocHelpers::PropertyData<double>(10, QMetaType::Double, QMC::DefaultPropertyFlags, 0),
        // property 'amplitud'
        QtMocHelpers::PropertyData<double>(11, QMetaType::Double, QMC::DefaultPropertyFlags, 0),
        // property 'iconSpacing'
        QtMocHelpers::PropertyData<int>(12, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'showReflection'
        QtMocHelpers::PropertyData<bool>(13, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'skinBlur'
        QtMocHelpers::PropertyData<bool>(14, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'skinBlurRadius'
        QtMocHelpers::PropertyData<int>(15, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'skinLiquidGelEffect'
        QtMocHelpers::PropertyData<bool>(16, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'skinRefractionStrength'
        QtMocHelpers::PropertyData<double>(17, QMetaType::Double, QMC::DefaultPropertyFlags, 0),
        // property 'skinRgbFringing'
        QtMocHelpers::PropertyData<double>(18, QMetaType::Double, QMC::DefaultPropertyFlags, 0),
        // property 'skinPositionTaskIndicator'
        QtMocHelpers::PropertyData<int>(19, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'showToolTips'
        QtMocHelpers::PropertyData<bool>(20, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'highlightWindows'
        QtMocHelpers::PropertyData<bool>(21, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'indicateAudioStreams'
        QtMocHelpers::PropertyData<bool>(22, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'interactiveMute'
        QtMocHelpers::PropertyData<bool>(23, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'tooltipControls'
        QtMocHelpers::PropertyData<bool>(24, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'minimizeActiveTaskOnClick'
        QtMocHelpers::PropertyData<bool>(25, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'middleClickAction'
        QtMocHelpers::PropertyData<int>(26, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'wheelEnabled'
        QtMocHelpers::PropertyData<int>(27, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'wheelSkipMinimized'
        QtMocHelpers::PropertyData<bool>(28, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'groupingStrategy'
        QtMocHelpers::PropertyData<int>(29, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'groupedTaskVisualization'
        QtMocHelpers::PropertyData<int>(30, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'sortingStrategy'
        QtMocHelpers::PropertyData<int>(31, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'showOnlyCurrentDesktop'
        QtMocHelpers::PropertyData<bool>(32, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'showOnlyCurrentActivity'
        QtMocHelpers::PropertyData<bool>(33, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'showOnlyCurrentScreen'
        QtMocHelpers::PropertyData<bool>(34, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'showOnlyMinimized'
        QtMocHelpers::PropertyData<bool>(35, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'unhideOnAttention'
        QtMocHelpers::PropertyData<bool>(36, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'skinLeftMargin'
        QtMocHelpers::PropertyData<int>(37, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'skinTopMargin'
        QtMocHelpers::PropertyData<int>(38, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'skinRightMargin'
        QtMocHelpers::PropertyData<int>(39, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'skinBottomMargin'
        QtMocHelpers::PropertyData<int>(40, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'skinOutsideLeftMargin'
        QtMocHelpers::PropertyData<int>(41, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'skinOutsideTopMargin'
        QtMocHelpers::PropertyData<int>(42, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'skinOutsideRightMargin'
        QtMocHelpers::PropertyData<int>(43, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'skinOutsideBottomMargin'
        QtMocHelpers::PropertyData<int>(44, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'availableSkins'
        QtMocHelpers::PropertyData<QStringList>(45, QMetaType::QStringList, QMC::DefaultPropertyFlags, 0),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DockManager, qt_meta_tag_ZN11DockManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DockManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11DockManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11DockManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11DockManagerE_t>.metaTypes,
    nullptr
} };

void DockManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DockManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->changed(); break;
        case 1: _t->refresh(); break;
        case 2: _t->set((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariant>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DockManager::*)()>(_a, &DockManager::changed, 0))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->skinName(); break;
        case 1: *reinterpret_cast<int*>(_v) = _t->iconSize(); break;
        case 2: *reinterpret_cast<double*>(_v) = _t->magnification(); break;
        case 3: *reinterpret_cast<double*>(_v) = _t->amplitud(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->iconSpacing(); break;
        case 5: *reinterpret_cast<bool*>(_v) = _t->showReflection(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->skinBlur(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->skinBlurRadius(); break;
        case 8: *reinterpret_cast<bool*>(_v) = _t->skinLiquidGelEffect(); break;
        case 9: *reinterpret_cast<double*>(_v) = _t->skinRefractionStrength(); break;
        case 10: *reinterpret_cast<double*>(_v) = _t->skinRgbFringing(); break;
        case 11: *reinterpret_cast<int*>(_v) = _t->skinPositionTaskIndicator(); break;
        case 12: *reinterpret_cast<bool*>(_v) = _t->showToolTips(); break;
        case 13: *reinterpret_cast<bool*>(_v) = _t->highlightWindows(); break;
        case 14: *reinterpret_cast<bool*>(_v) = _t->indicateAudioStreams(); break;
        case 15: *reinterpret_cast<bool*>(_v) = _t->interactiveMute(); break;
        case 16: *reinterpret_cast<bool*>(_v) = _t->tooltipControls(); break;
        case 17: *reinterpret_cast<bool*>(_v) = _t->minimizeActiveTaskOnClick(); break;
        case 18: *reinterpret_cast<int*>(_v) = _t->middleClickAction(); break;
        case 19: *reinterpret_cast<int*>(_v) = _t->wheelEnabled(); break;
        case 20: *reinterpret_cast<bool*>(_v) = _t->wheelSkipMinimized(); break;
        case 21: *reinterpret_cast<int*>(_v) = _t->groupingStrategy(); break;
        case 22: *reinterpret_cast<int*>(_v) = _t->groupedTaskVisualization(); break;
        case 23: *reinterpret_cast<int*>(_v) = _t->sortingStrategy(); break;
        case 24: *reinterpret_cast<bool*>(_v) = _t->showOnlyCurrentDesktop(); break;
        case 25: *reinterpret_cast<bool*>(_v) = _t->showOnlyCurrentActivity(); break;
        case 26: *reinterpret_cast<bool*>(_v) = _t->showOnlyCurrentScreen(); break;
        case 27: *reinterpret_cast<bool*>(_v) = _t->showOnlyMinimized(); break;
        case 28: *reinterpret_cast<bool*>(_v) = _t->unhideOnAttention(); break;
        case 29: *reinterpret_cast<int*>(_v) = _t->skinLeftMargin(); break;
        case 30: *reinterpret_cast<int*>(_v) = _t->skinTopMargin(); break;
        case 31: *reinterpret_cast<int*>(_v) = _t->skinRightMargin(); break;
        case 32: *reinterpret_cast<int*>(_v) = _t->skinBottomMargin(); break;
        case 33: *reinterpret_cast<int*>(_v) = _t->skinOutsideLeftMargin(); break;
        case 34: *reinterpret_cast<int*>(_v) = _t->skinOutsideTopMargin(); break;
        case 35: *reinterpret_cast<int*>(_v) = _t->skinOutsideRightMargin(); break;
        case 36: *reinterpret_cast<int*>(_v) = _t->skinOutsideBottomMargin(); break;
        case 37: *reinterpret_cast<QStringList*>(_v) = _t->availableSkins(); break;
        default: break;
        }
    }
}

const QMetaObject *DockManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DockManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11DockManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DockManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 38;
    }
    return _id;
}

// SIGNAL 0
void DockManager::changed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
