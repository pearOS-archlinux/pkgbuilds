#!/bin/bash

# ==============================================================================
# CONFIGURATOR TOUCHPAD KDE (WAYLAND) - v4.0 (Hardware Specific)
# ==============================================================================
# Adaptat pentru proprietățile exacte: scrollTwoFinger, scrollEdge, middleEmulation
# Salvează starea în /usr/share/extras/settings/touchpad/config.json
# ==============================================================================

JSON_DIR="/usr/share/extras/settings/touchpad"
JSON_FILE="$JSON_DIR/config.json"

# --- Funcția de Help ---
show_help() {
    echo "Utilizare: $0 [OPȚIUNI]"
    echo "Opțiuni (true/false):"
    echo "  -e [true|false]  Enable Touchpad"
    echo "  -n [true|false]  Natural Scroll"
    echo "  -t [true|false]  Tap To Click"
    echo "  -w [true|false]  Disable While Typing"
    echo "  -l [true|false]  Left Handed"
    echo "  -m [true|false]  Middle Emulation (Click 3 degete = Paste)"
    echo "  -g [true|false]  Tap And Drag"
    echo "  -k [true|false]  Tap Drag Lock"
    echo ""
    echo "Opțiuni Numerice:"
    echo "  -a [număr]       Accelerație (-1.0 la 1.0)"
    echo "  -f [număr]       Scroll Factor (Viteză scroll, ex: 1.0, 2.5)"
    echo ""
    echo "Metode (Selectare mod):"
    echo "  -s [1|2]         Scroll Method: 1=Two-Finger (Standard), 2=Edge (Margine)"
    echo "  -c [1|2]         Click Method:  1=ButtonAreas (Mecanic), 2=Clickfinger (Modern)"
    echo "  -p [1|2]         Accel Profile: 1=Flat (Fără accel), 2=Adaptive (Standard)"
    exit 0
}

# --- 1. Definire valori implicite ---
DEF_ENABLED="true"
DEF_NATURAL_SCROLL="true"
DEF_TAP_TO_CLICK="true"
DEF_DISABLE_WHILE_TYPING="true"
DEF_DISABLE_ON_EXT_MOUSE="false"
DEF_LEFT_HANDED="false"
DEF_MIDDLE_EMULATION="false"
DEF_LMR_TAP_MAP="false"
DEF_TAP_AND_DRAG="true"
DEF_TAP_DRAG_LOCK="false"
DEF_PTR_ACCEL="0.000"
DEF_PTR_PROFILE="2"      # 2 = Adaptive
DEF_SCROLL_FACTOR="1.0"
DEF_CLICK_METHOD="2"     # 2 = Clickfinger (Modern)
DEF_SCROLL_METHOD="1"    # 1 = TwoFinger

# Inițializăm variabilele
CUR_ENABLED=$DEF_ENABLED
CUR_NATURAL_SCROLL=$DEF_NATURAL_SCROLL
CUR_TAP_TO_CLICK=$DEF_TAP_TO_CLICK
CUR_DISABLE_WHILE_TYPING=$DEF_DISABLE_WHILE_TYPING
CUR_DISABLE_ON_EXT_MOUSE=$DEF_DISABLE_ON_EXT_MOUSE
CUR_LEFT_HANDED=$DEF_LEFT_HANDED
CUR_MIDDLE_EMULATION=$DEF_MIDDLE_EMULATION
CUR_LMR_TAP_MAP=$DEF_LMR_TAP_MAP
CUR_TAP_AND_DRAG=$DEF_TAP_AND_DRAG
CUR_TAP_DRAG_LOCK=$DEF_TAP_DRAG_LOCK
CUR_PTR_ACCEL=$DEF_PTR_ACCEL
CUR_PTR_PROFILE=$DEF_PTR_PROFILE
CUR_SCROLL_FACTOR=$DEF_SCROLL_FACTOR
CUR_CLICK_METHOD=$DEF_CLICK_METHOD
CUR_SCROLL_METHOD=$DEF_SCROLL_METHOD

# --- 2. Citește JSON ---
get_json_val() {
    key=$1; default=$2
    if [ -f "$JSON_FILE" ]; then
        val=$(grep -o "\"$key\": [^,}]*" "$JSON_FILE" | head -n1 | cut -d' ' -f2 | tr -d '"')
        echo "${val:-$default}"
    else echo "$default"; fi
}

if [ -f "$JSON_FILE" ]; then
    CUR_ENABLED=$(get_json_val "Enabled" $DEF_ENABLED)
    CUR_NATURAL_SCROLL=$(get_json_val "NaturalScroll" $DEF_NATURAL_SCROLL)
    CUR_TAP_TO_CLICK=$(get_json_val "TapToClick" $DEF_TAP_TO_CLICK)
    CUR_DISABLE_WHILE_TYPING=$(get_json_val "DisableWhileTyping" $DEF_DISABLE_WHILE_TYPING)
    CUR_DISABLE_ON_EXT_MOUSE=$(get_json_val "DisableEventsOnExternalMouse" $DEF_DISABLE_ON_EXT_MOUSE)
    CUR_LEFT_HANDED=$(get_json_val "LeftHanded" $DEF_LEFT_HANDED)
    CUR_MIDDLE_EMULATION=$(get_json_val "MiddleEmulation" $DEF_MIDDLE_EMULATION)
    CUR_LMR_TAP_MAP=$(get_json_val "LmrTapButtonMap" $DEF_LMR_TAP_MAP)
    CUR_TAP_AND_DRAG=$(get_json_val "TapAndDrag" $DEF_TAP_AND_DRAG)
    CUR_TAP_DRAG_LOCK=$(get_json_val "TapDragLock" $DEF_TAP_DRAG_LOCK)
    CUR_PTR_ACCEL=$(get_json_val "PointerAcceleration" $DEF_PTR_ACCEL)
    CUR_PTR_PROFILE=$(get_json_val "PointerAccelerationProfile" $DEF_PTR_PROFILE)
    CUR_SCROLL_FACTOR=$(get_json_val "ScrollFactor" $DEF_SCROLL_FACTOR)
    CUR_CLICK_METHOD=$(get_json_val "ClickMethod" $DEF_CLICK_METHOD)
    CUR_SCROLL_METHOD=$(get_json_val "ScrollMethod" $DEF_SCROLL_METHOD)
fi

# --- 3. Argumente ---
while getopts "e:n:t:w:x:l:m:b:g:k:a:p:f:c:s:h" opt; do
  case $opt in
    e) NEW_ENABLED="$OPTARG" ;;
    n) NEW_NATURAL_SCROLL="$OPTARG" ;;
    t) NEW_TAP_TO_CLICK="$OPTARG" ;;
    w) NEW_DISABLE_WHILE_TYPING="$OPTARG" ;;
    x) NEW_DISABLE_ON_EXT_MOUSE="$OPTARG" ;;
    l) NEW_LEFT_HANDED="$OPTARG" ;;
    m) NEW_MIDDLE_EMULATION="$OPTARG" ;;
    b) NEW_LMR_TAP_MAP="$OPTARG" ;;
    g) NEW_TAP_AND_DRAG="$OPTARG" ;;
    k) NEW_TAP_DRAG_LOCK="$OPTARG" ;;
    a) NEW_PTR_ACCEL="$OPTARG" ;;
    p) NEW_PTR_PROFILE="$OPTARG" ;;
    f) NEW_SCROLL_FACTOR="$OPTARG" ;;
    c) NEW_CLICK_METHOD="$OPTARG" ;;
    s) NEW_SCROLL_METHOD="$OPTARG" ;;
    h) show_help ;;
    \?) exit 1 ;;
  esac
done

# Valori Finale
FIN_ENABLED=${NEW_ENABLED:-$CUR_ENABLED}
FIN_NATURAL_SCROLL=${NEW_NATURAL_SCROLL:-$CUR_NATURAL_SCROLL}
FIN_TAP_TO_CLICK=${NEW_TAP_TO_CLICK:-$CUR_TAP_TO_CLICK}
FIN_DISABLE_WHILE_TYPING=${NEW_DISABLE_WHILE_TYPING:-$CUR_DISABLE_WHILE_TYPING}
FIN_DISABLE_ON_EXT_MOUSE=${NEW_DISABLE_ON_EXT_MOUSE:-$CUR_DISABLE_ON_EXT_MOUSE}
FIN_LEFT_HANDED=${NEW_LEFT_HANDED:-$CUR_LEFT_HANDED}
FIN_MIDDLE_EMULATION=${NEW_MIDDLE_EMULATION:-$CUR_MIDDLE_EMULATION}
FIN_LMR_TAP_MAP=${NEW_LMR_TAP_MAP:-$CUR_LMR_TAP_MAP}
FIN_TAP_AND_DRAG=${NEW_TAP_AND_DRAG:-$CUR_TAP_AND_DRAG}
FIN_TAP_DRAG_LOCK=${NEW_TAP_DRAG_LOCK:-$CUR_TAP_DRAG_LOCK}
FIN_PTR_ACCEL=${NEW_PTR_ACCEL:-$CUR_PTR_ACCEL}
FIN_PTR_PROFILE=${NEW_PTR_PROFILE:-$CUR_PTR_PROFILE}
FIN_SCROLL_FACTOR=${NEW_SCROLL_FACTOR:-$CUR_SCROLL_FACTOR}
FIN_CLICK_METHOD=${NEW_CLICK_METHOD:-$CUR_CLICK_METHOD}
FIN_SCROLL_METHOD=${NEW_SCROLL_METHOD:-$CUR_SCROLL_METHOD}

# --- 4. Scrie JSON ---
if [ ! -d "$JSON_DIR" ]; then mkdir -p "$JSON_DIR" 2>/dev/null; fi
JSON_CONTENT="{
  \"Enabled\": $FIN_ENABLED,
  \"NaturalScroll\": $FIN_NATURAL_SCROLL,
  \"TapToClick\": $FIN_TAP_TO_CLICK,
  \"DisableWhileTyping\": $FIN_DISABLE_WHILE_TYPING,
  \"DisableEventsOnExternalMouse\": $FIN_DISABLE_ON_EXT_MOUSE,
  \"LeftHanded\": $FIN_LEFT_HANDED,
  \"MiddleEmulation\": $FIN_MIDDLE_EMULATION,
  \"LmrTapButtonMap\": $FIN_LMR_TAP_MAP,
  \"TapAndDrag\": $FIN_TAP_AND_DRAG,
  \"TapDragLock\": $FIN_TAP_DRAG_LOCK,
  \"PointerAcceleration\": $FIN_PTR_ACCEL,
  \"PointerAccelerationProfile\": $FIN_PTR_PROFILE,
  \"ScrollFactor\": $FIN_SCROLL_FACTOR,
  \"ClickMethod\": $FIN_CLICK_METHOD,
  \"ScrollMethod\": $FIN_SCROLL_METHOD
}"
if echo "$JSON_CONTENT" > "$JSON_FILE" 2>/dev/null; then :; else
    echo "⚠️  Eroare scriere JSON. Lipsesc permisiuni pe folder."
fi

# --- 5. Aplică Hardware (Conversie Logică) ---
if command -v qdbus6 &> /dev/null; then QDBUS_CMD="qdbus6"; else QDBUS_CMD="qdbus"; fi

# Logică: Pointer Acceleration Profile
if [ "$FIN_PTR_PROFILE" == "1" ]; then ACCEL_FLAT="true"; ACCEL_ADAPTIVE="false"
else ACCEL_FLAT="false"; ACCEL_ADAPTIVE="true"; fi

# Logică: Scroll Method (1=TwoFinger, 2=Edge)
if [ "$FIN_SCROLL_METHOD" == "2" ]; then SCROLL_TWO="false"; SCROLL_EDGE="true"
else SCROLL_TWO="true"; SCROLL_EDGE="false"; fi

# Logică: Click Method (1=Areas, 2=Clickfinger)
if [ "$FIN_CLICK_METHOD" == "1" ]; then CLICK_AREAS="true"; CLICK_FINGER="false"
else CLICK_AREAS="false"; CLICK_FINGER="true"; fi

for path in $($QDBUS_CMD org.kde.KWin | grep "/org/kde/KWin/InputDevice/event"); do
    event_num=$(echo "$path" | grep -oE '[0-9]+$')
    dev_node="/dev/input/event$event_num"
    is_touchpad=$(udevadm info -n "$dev_node" 2>/dev/null | grep "ID_INPUT_TOUCHPAD=1")
    is_mouse=$(udevadm info -n "$dev_node" 2>/dev/null | grep "ID_INPUT_MOUSE=1")

    # Helper pentru setare silentioasa
    set_prop() { $QDBUS_CMD org.kde.KWin "$path" org.freedesktop.DBus.Properties.Set org.kde.KWin.InputDevice "$1" "$2" 2>/dev/null; }

    if [ -n "$is_touchpad" ]; then
        # Booleans simple (Nume corectate conform listei tale)
        set_prop enabled "$FIN_ENABLED"
        set_prop naturalScroll "$FIN_NATURAL_SCROLL"
        set_prop tapToClick "$FIN_TAP_TO_CLICK"
        set_prop disableWhileTyping "$FIN_DISABLE_WHILE_TYPING"
        set_prop disableEventsOnExternalMouse "$FIN_DISABLE_ON_EXT_MOUSE"
        set_prop leftHanded "$FIN_LEFT_HANDED"
        set_prop middleEmulation "$FIN_MIDDLE_EMULATION"  # Nume corectat!
        set_prop lmrTapButtonMap "$FIN_LMR_TAP_MAP"
        set_prop tapAndDrag "$FIN_TAP_AND_DRAG"
        set_prop tapDragLock "$FIN_TAP_DRAG_LOCK"
        
        # Numerice simple
        set_prop pointerAcceleration "$FIN_PTR_ACCEL"
        set_prop scrollFactor "$FIN_SCROLL_FACTOR"
        
        # Proprietăți complexe (Boolean switches)
        set_prop pointerAccelerationProfileFlat "$ACCEL_FLAT"
        set_prop pointerAccelerationProfileAdaptive "$ACCEL_ADAPTIVE"
        
        set_prop scrollTwoFinger "$SCROLL_TWO"
        set_prop scrollEdge "$SCROLL_EDGE"
        
        set_prop clickMethodAreas "$CLICK_AREAS"
        set_prop clickMethodClickfinger "$CLICK_FINGER"

    elif [ -n "$is_mouse" ]; then
        set_prop naturalScroll false
    fi
done