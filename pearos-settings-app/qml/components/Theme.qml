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
    readonly property color textSecondary: dark ? "#b2ffffff"         : "#666666"
    readonly property color textTertiary:  dark ? Qt.rgba(1,1,1,0.5) : Qt.rgba(0,0,0,0.5)

    // Dividers / borders
    readonly property color divider: dark ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1)
    readonly property color border:  dark ? "#2F343A"           : "#e0e0e0"

    // Interactive states
    readonly property color activeBg: dark ? Qt.rgba(1,1,1,0.15)           : Qt.rgba(0.067,0.067,0.067,0.15)
    readonly property color hoverBg:  dark ? Qt.rgba(1,1,1,0.1)            : Qt.rgba(0.067,0.067,0.067,0.08)

    // Accent — reads the name from AppearanceManager, maps to hex (same table as Electron)
    readonly property var _accentMap: ({
        'purple':      '#8B5CF6',
        'magenta':     '#EC4899',
        'orange':      '#F97316',
        'yellow':      '#EAB308',
        'green':       '#22C55E',
        'azul':        '#06B6D4',
        'blue':        '#3B82F6',
        'lila':        '#A855F7',
        'dark-purple': '#6B21A8',
        'grey':        '#6B7280'
    })
    readonly property string accentHex: _accentMap[Appearance.accent] || '#3B82F6'
    readonly property color  accent:    Qt.color(accentHex)

    // Toggle track (off state)
    readonly property color toggleOff: dark ? Qt.rgba(1,1,1,0.2) : "#cccccc"
}
