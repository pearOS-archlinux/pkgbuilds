pragma Singleton
import QtQuick

QtObject {
    readonly property bool dark: Qt.styleHints.colorScheme === Qt.ColorScheme.Dark

    // Backgrounds
    readonly property color bg:        dark ? "#1D232A" : "#ffffff"
    readonly property color bgCard:    dark ? "#242A31" : "#F7F7F7"
    readonly property color bgSidenav: dark ? "#1A1F24" : "#F7F8F8"
    readonly property color bgSearch:  dark ? "#2A3040" : "#e0e0e0"

    // Text
    readonly property color textPrimary:   dark ? "#ffffff"           : "#111111"
    readonly property color textSecondary: dark ? "#ffffffb2"         : "#666666"
    readonly property color textTertiary:  dark ? Qt.rgba(1,1,1,0.5) : Qt.rgba(0,0,0,0.5)

    // Dividers / borders
    readonly property color divider: dark ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1)
    readonly property color border:  dark ? "#2F343A"           : "#e0e0e0"

    // Interactive states
    readonly property color activeBg: dark ? Qt.rgba(1,1,1,0.15)           : Qt.rgba(0.067,0.067,0.067,0.15)
    readonly property color hoverBg:  dark ? Qt.rgba(1,1,1,0.1)            : Qt.rgba(0.067,0.067,0.067,0.08)

    // Accent
    readonly property color accent: "#3B82F6"
}
