#ifndef PLASMATYPESSHIM_H
#define PLASMATYPESSHIM_H

#include <QObject>
#include <QQmlEngine>

// Stand-in for org.kde.plasma.core's `PlasmaCore.Types` enum namespace --
// values copied verbatim from the installed
// /usr/include/Plasma/plasma/plasma.h (Plasma::Types::FormFactor/Location/
// ItemStatus/BackgroundHints/ContainmentDisplayHint), not guessed. A plain
// QML `property int Foo` can't use PascalCase names (QML requires
// lowerCamelCase for declared properties), so this uses Q_NAMESPACE +
// Q_ENUM_NS like the real Plasma::Types does, registered as QML singleton
// type "Types" -- callers only need to change their import line
// (`import org.pearos.shellshim as PlasmaCore`), every `PlasmaCore.Types.Foo`
// reference keeps working unchanged.
namespace PlasmaTypesShim
{
Q_NAMESPACE
QML_NAMED_ELEMENT(Types)

enum FormFactor {
    Planar = 0,
    MediaCenter,
    Horizontal,
    Vertical,
    Application,
};
Q_ENUM_NS(FormFactor)

enum Location {
    Floating = 0,
    Desktop,
    FullScreen,
    TopEdge,
    BottomEdge,
    LeftEdge,
    RightEdge,
};
Q_ENUM_NS(Location)

enum ItemStatus {
    UnknownStatus = 0,
    PassiveStatus = 1,
    ActiveStatus = 2,
    NeedsAttentionStatus = 3,
    RequiresAttentionStatus = 4,
    AcceptingInputStatus = 5,
    HiddenStatus = 6,
};
Q_ENUM_NS(ItemStatus)

enum BackgroundHints {
    NoBackground = 0,
    StandardBackground = 1,
    TranslucentBackground = 2,
    ShadowBackground = 4,
    ConfigurableBackground = 8,
    DefaultBackground = StandardBackground,
};
Q_ENUM_NS(BackgroundHints)

enum ContainmentDisplayHint {
    NoContainmentDisplayHint = 0,
    ContainmentDrawsPlasmoidHeading = 1,
    ContainmentForcesSquarePlasmoids = 2,
    ContainmentPrefersOpaqueBackground = 4,
    ContainmentPrefersFloatingApplets = 8,
};
Q_ENUM_NS(ContainmentDisplayHint)

} // namespace PlasmaTypesShim

#endif // PLASMATYPESSHIM_H
