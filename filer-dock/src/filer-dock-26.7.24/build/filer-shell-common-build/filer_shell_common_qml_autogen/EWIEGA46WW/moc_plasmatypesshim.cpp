/****************************************************************************
** Meta object code from reading C++ file 'plasmatypesshim.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../filer-shell-common/plasmatypesshim.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'plasmatypesshim.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15PlasmaTypesShimE_t {};
} // unnamed namespace

template <> constexpr inline auto PlasmaTypesShim::qt_create_metaobjectdata<qt_meta_tag_ZN15PlasmaTypesShimE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PlasmaTypesShim",
        "QML.Element",
        "Types",
        "FormFactor",
        "Planar",
        "MediaCenter",
        "Horizontal",
        "Vertical",
        "Application",
        "Location",
        "Floating",
        "Desktop",
        "FullScreen",
        "TopEdge",
        "BottomEdge",
        "LeftEdge",
        "RightEdge",
        "ItemStatus",
        "UnknownStatus",
        "PassiveStatus",
        "ActiveStatus",
        "NeedsAttentionStatus",
        "RequiresAttentionStatus",
        "AcceptingInputStatus",
        "HiddenStatus",
        "BackgroundHints",
        "NoBackground",
        "StandardBackground",
        "TranslucentBackground",
        "ShadowBackground",
        "ConfigurableBackground",
        "DefaultBackground",
        "ContainmentDisplayHint",
        "NoContainmentDisplayHint",
        "ContainmentDrawsPlasmoidHeading",
        "ContainmentForcesSquarePlasmoids",
        "ContainmentPrefersOpaqueBackground",
        "ContainmentPrefersFloatingApplets"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'FormFactor'
        QtMocHelpers::EnumData<FormFactor>(3, 3, QMC::EnumFlags{}).add({
            {    4, FormFactor::Planar },
            {    5, FormFactor::MediaCenter },
            {    6, FormFactor::Horizontal },
            {    7, FormFactor::Vertical },
            {    8, FormFactor::Application },
        }),
        // enum 'Location'
        QtMocHelpers::EnumData<Location>(9, 9, QMC::EnumFlags{}).add({
            {   10, Location::Floating },
            {   11, Location::Desktop },
            {   12, Location::FullScreen },
            {   13, Location::TopEdge },
            {   14, Location::BottomEdge },
            {   15, Location::LeftEdge },
            {   16, Location::RightEdge },
        }),
        // enum 'ItemStatus'
        QtMocHelpers::EnumData<ItemStatus>(17, 17, QMC::EnumFlags{}).add({
            {   18, ItemStatus::UnknownStatus },
            {   19, ItemStatus::PassiveStatus },
            {   20, ItemStatus::ActiveStatus },
            {   21, ItemStatus::NeedsAttentionStatus },
            {   22, ItemStatus::RequiresAttentionStatus },
            {   23, ItemStatus::AcceptingInputStatus },
            {   24, ItemStatus::HiddenStatus },
        }),
        // enum 'BackgroundHints'
        QtMocHelpers::EnumData<BackgroundHints>(25, 25, QMC::EnumFlags{}).add({
            {   26, BackgroundHints::NoBackground },
            {   27, BackgroundHints::StandardBackground },
            {   28, BackgroundHints::TranslucentBackground },
            {   29, BackgroundHints::ShadowBackground },
            {   30, BackgroundHints::ConfigurableBackground },
            {   31, BackgroundHints::DefaultBackground },
        }),
        // enum 'ContainmentDisplayHint'
        QtMocHelpers::EnumData<ContainmentDisplayHint>(32, 32, QMC::EnumFlags{}).add({
            {   33, ContainmentDisplayHint::NoContainmentDisplayHint },
            {   34, ContainmentDisplayHint::ContainmentDrawsPlasmoidHeading },
            {   35, ContainmentDisplayHint::ContainmentForcesSquarePlasmoids },
            {   36, ContainmentDisplayHint::ContainmentPrefersOpaqueBackground },
            {   37, ContainmentDisplayHint::ContainmentPrefersFloatingApplets },
        }),
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    2 },
    });
    return QtMocHelpers::metaObjectData<void, qt_meta_tag_ZN15PlasmaTypesShimE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}

static constexpr auto qt_staticMetaObjectContent_ZN15PlasmaTypesShimE =
    PlasmaTypesShim::qt_create_metaobjectdata<qt_meta_tag_ZN15PlasmaTypesShimE_t>();
static constexpr auto qt_staticMetaObjectStaticContent_ZN15PlasmaTypesShimE =
    qt_staticMetaObjectContent_ZN15PlasmaTypesShimE.staticData;
static constexpr auto qt_staticMetaObjectRelocatingContent_ZN15PlasmaTypesShimE =
    qt_staticMetaObjectContent_ZN15PlasmaTypesShimE.relocatingData;

Q_CONSTINIT const QMetaObject PlasmaTypesShim::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent_ZN15PlasmaTypesShimE.stringdata,
    qt_staticMetaObjectStaticContent_ZN15PlasmaTypesShimE.data,
    nullptr,
    nullptr,
    qt_staticMetaObjectRelocatingContent_ZN15PlasmaTypesShimE.metaTypes,
    nullptr
} };

QT_WARNING_POP
