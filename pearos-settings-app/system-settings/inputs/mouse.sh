#!/bin/bash

# ==============================================================================
# CONFIGURATOR MOUSE KDE (WAYLAND) - Full Option
# ==============================================================================
# Configurează orice dispozitiv de tip Pointer care NU este Touchpad.
# Salvează starea în /usr/share/extras/settings/mouse/config.json
# ==============================================================================

JSON_DIR="/usr/share/extras/settings/mouse"
JSON_FILE="$JSON_DIR/config.json"

# --- Funcția de Help ---
show_help() {
    echo "Utilizare: $0 [OPȚIUNI]"
    echo "Opțiuni (true/false):"
    echo "  -e [true|false]  Enabled (Activare Mouse)"
    echo "  -l [true|false]  Left Handed (Stângaci)"
    echo "  -m [true|false]  Middle Button Emulation"
    echo "  -n [true|false]  Natural Scroll"
    echo ""
    echo "Opțiuni Numerice:"
    echo "  -a [număr]       Accelerație (-1.000 la 1.000)"
    echo "  -p [1|2]         Profile: 1=Flat, 2=Adaptive (Standard)"
    echo "  -f [număr]       Scroll Factor (Viteză scroll, ex: 1)"
    echo "  -s [0]           Scroll Method (0 = Standard/Wheel)"
    exit 0
}

# --- 1. Definire valori implicite (Lista ta) ---
DEF_ENABLED="true"
DEF_LEFT_HANDED="false"
DEF_MIDDLE_EMULATION="false"
DEF_NATURAL_SCROLL="false"
DEF_PTR_ACCEL="0.000"
DEF_PTR_PROFILE="2"      # 2 = Adaptive
DEF_SCROLL_FACTOR="1"
DEF_SCROLL_METHOD="0"    # 0 = Standard

# Inițializare variabile
CUR_ENABLED=$DEF_ENABLED
CUR_LEFT_HANDED=$DEF_LEFT_HANDED
CUR_MIDDLE_EMULATION=$DEF_MIDDLE_EMULATION
CUR_NATURAL_SCROLL=$DEF_NATURAL_SCROLL
CUR_PTR_ACCEL=$DEF_PTR_ACCEL
CUR_PTR_PROFILE=$DEF_PTR_PROFILE
CUR_SCROLL_FACTOR=$DEF_SCROLL_FACTOR
CUR_SCROLL_METHOD=$DEF_SCROLL_METHOD

# --- 2. Citește JSON (dacă există) ---
get_json_val() {
    key=$1; default=$2
    if [ -f "$JSON_FILE" ]; then
        val=$(grep -o "\"$key\": [^,}]*" "$JSON_FILE" | head -n1 | cut -d' ' -f2 | tr -d '"')
        echo "${val:-$default}"
    else echo "$default"; fi
}

if [ -f "$JSON_FILE" ]; then
    CUR_ENABLED=$(get_json_val "Enabled" $DEF_ENABLED)
    CUR_LEFT_HANDED=$(get_json_val "LeftHanded" $DEF_LEFT_HANDED)
    CUR_MIDDLE_EMULATION=$(get_json_val "MiddleButtonEmulation" $DEF_MIDDLE_EMULATION)
    CUR_NATURAL_SCROLL=$(get_json_val "NaturalScroll" $DEF_NATURAL_SCROLL)
    CUR_PTR_ACCEL=$(get_json_val "PointerAcceleration" $DEF_PTR_ACCEL)
    CUR_PTR_PROFILE=$(get_json_val "PointerAccelerationProfile" $DEF_PTR_PROFILE)
    CUR_SCROLL_FACTOR=$(get_json_val "ScrollFactor" $DEF_SCROLL_FACTOR)
    CUR_SCROLL_METHOD=$(get_json_val "ScrollMethod" $DEF_SCROLL_METHOD)
fi

# --- 3. Procesare Argumente ---
while getopts "e:l:m:n:a:p:f:s:h" opt; do
  case $opt in
    e) NEW_ENABLED="$OPTARG" ;;
    l) NEW_LEFT_HANDED="$OPTARG" ;;
    m) NEW_MIDDLE_EMULATION="$OPTARG" ;;
    n) NEW_NATURAL_SCROLL="$OPTARG" ;;
    a) NEW_PTR_ACCEL="$OPTARG" ;;
    p) NEW_PTR_PROFILE="$OPTARG" ;;
    f) NEW_SCROLL_FACTOR="$OPTARG" ;;
    s) NEW_SCROLL_METHOD="$OPTARG" ;;
    h) show_help ;;
    \?) exit 1 ;;
  esac
done

# Valori Finale
FIN_ENABLED=${NEW_ENABLED:-$CUR_ENABLED}
FIN_LEFT_HANDED=${NEW_LEFT_HANDED:-$CUR_LEFT_HANDED}
FIN_MIDDLE_EMULATION=${NEW_MIDDLE_EMULATION:-$CUR_MIDDLE_EMULATION}
FIN_NATURAL_SCROLL=${NEW_NATURAL_SCROLL:-$CUR_NATURAL_SCROLL}
FIN_PTR_ACCEL=${NEW_PTR_ACCEL:-$CUR_PTR_ACCEL}
FIN_PTR_PROFILE=${NEW_PTR_PROFILE:-$CUR_PTR_PROFILE}
FIN_SCROLL_FACTOR=${NEW_SCROLL_FACTOR:-$CUR_SCROLL_FACTOR}
FIN_SCROLL_METHOD=${NEW_SCROLL_METHOD:-$CUR_SCROLL_METHOD}

# --- 4. Scrie JSON ---
if [ ! -d "$JSON_DIR" ]; then mkdir -p "$JSON_DIR" 2>/dev/null; fi
JSON_CONTENT="{
  \"Enabled\": $FIN_ENABLED,
  \"LeftHanded\": $FIN_LEFT_HANDED,
  \"MiddleButtonEmulation\": $FIN_MIDDLE_EMULATION,
  \"NaturalScroll\": $FIN_NATURAL_SCROLL,
  \"PointerAcceleration\": $FIN_PTR_ACCEL,
  \"PointerAccelerationProfile\": $FIN_PTR_PROFILE,
  \"ScrollFactor\": $FIN_SCROLL_FACTOR,
  \"ScrollMethod\": $FIN_SCROLL_METHOD
}"
if echo "$JSON_CONTENT" > "$JSON_FILE" 2>/dev/null; then :; else
    echo "⚠️  Eroare scriere JSON în $JSON_FILE. Lipsesc permisiuni."
fi

# --- 5. Aplică Hardware (Mouse ONLY) ---
if command -v qdbus6 &> /dev/null; then QDBUS_CMD="qdbus6"; else QDBUS_CMD="qdbus"; fi

# Logică Profil Accelerare (1=Flat, 2=Adaptive)
if [ "$FIN_PTR_PROFILE" == "1" ]; then ACCEL_FLAT="true"; ACCEL_ADAPTIVE="false"
else ACCEL_FLAT="false"; ACCEL_ADAPTIVE="true"; fi

# Logică Scroll Method (Mouse-ul clasic nu are edge/2-finger scroll)
SCROLL_TWO="false"; SCROLL_EDGE="false" 

for path in $($QDBUS_CMD org.kde.KWin | grep "/org/kde/KWin/InputDevice/event"); do
    event_num=$(echo "$path" | grep -oE '[0-9]+$')
    dev_node="/dev/input/event$event_num"
    
    # Identificare Strictă: E Mouse DAR NU e Touchpad
    is_touchpad=$(udevadm info -n "$dev_node" 2>/dev/null | grep "ID_INPUT_TOUCHPAD=1")
    is_mouse=$(udevadm info -n "$dev_node" 2>/dev/null | grep "ID_INPUT_MOUSE=1")

    # Helper setare silentioasa
    set_prop() { $QDBUS_CMD org.kde.KWin "$path" org.freedesktop.DBus.Properties.Set org.kde.KWin.InputDevice "$1" "$2" 2>/dev/null; }

    if [ -n "$is_mouse" ] && [ -z "$is_touchpad" ]; then
        # Nou: Enabled
        set_prop enabled "$FIN_ENABLED"

        # Booleans
        set_prop leftHanded "$FIN_LEFT_HANDED"
        set_prop middleEmulation "$FIN_MIDDLE_EMULATION"
        set_prop naturalScroll "$FIN_NATURAL_SCROLL"
        
        # Numerice
        set_prop pointerAcceleration "$FIN_PTR_ACCEL"
        set_prop scrollFactor "$FIN_SCROLL_FACTOR"
        
        # Profile
        set_prop pointerAccelerationProfileFlat "$ACCEL_FLAT"
        set_prop pointerAccelerationProfileAdaptive "$ACCEL_ADAPTIVE"
        
        # Scroll Method (Dezactivam metodele tactile pentru mouse)
        set_prop scrollTwoFinger "$SCROLL_TWO"
        set_prop scrollEdge "$SCROLL_EDGE"
    fi
done