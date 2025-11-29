#!/usr/bin/env bash
# KDE theme switcher & lister (dark/light)
# Works on KDE Plasma 5/6. Non-interactive, safe to run repeatedly.
set -euo pipefail

# ------------------------------
# Static defaults (config obligatoriu prin config.json)
# ------------------------------
# Window decorations (KWin) - folosim Aurorae
KWIN_DECORATION_PLUGIN="org.kde.kwin.aurorae"

# ------------------------------
# Helpers
# ------------------------------
have() { command -v "$1" >/dev/null 2>&1; }

msg() { printf '[kde-theme-switch] %s\n' "$*"; }

SCRIPT_DIR="$(dirname "$0")"

# Local first, then system-wide defaults under /usr/share/extras/pearos-themesw
if [ -f "$SCRIPT_DIR/config.json" ]; then
  CONFIG_JSON="$SCRIPT_DIR/config.json"
elif [ -f "/usr/share/extras/pearos-themesw/config.json" ]; then
  CONFIG_JSON="/usr/share/extras/pearos-themesw/config.json"
else
  CONFIG_JSON="$SCRIPT_DIR/config.json"  # will trigger die_config if actually missing
fi

if [ -d "$SCRIPT_DIR/colors" ]; then
  COLORS_DIR="$SCRIPT_DIR/colors"
elif [ -d "/usr/share/extras/pearos-themesw/colors" ]; then
  COLORS_DIR="/usr/share/extras/pearos-themesw/colors"
else
  COLORS_DIR="$SCRIPT_DIR/colors"
fi

# ------------------------------
# Wallpaper helpers (pearOS only)
# ------------------------------
set_wallpaper_if_pearos() {
  local mode="$1"
  # Determine current wallpaper using Plasma config
  local current
  current="$(grep "Image=" ~/.config/plasma-org.kde.plasma.desktop-appletsrc 2>/dev/null \
    | grep -v "Image=true" \
    | sed 's/.*Image=//' \
    | tail -n 1)"
  # Normalize file:// URIs to plain paths
  current="${current#file://}"
  msg "DEBUG wallpaper current: '${current}'"
  [ -z "$current" ] && return 0
  local wp_dark="/usr/share/wallpapers/pearOS-dark/default.jpg"
  local wp_light="/usr/share/wallpapers/pearOS/default.jpg"
  # Only touch wallpaper if current one is one of the pearOS defaults
  if [ "$current" != "$wp_dark" ] && [ "$current" != "$wp_light" ]; then
    msg "DEBUG wallpaper is not a pearOS default (skip wallpaper change)"
    return 0
  fi
  local target="$wp_dark"
  if [ "$mode" = "light" ]; then
    target="$wp_light"
  fi
  msg "DEBUG mode: '$mode', target wallpaper: '$target'"
  msg "DEBUG plasma-apply-wallpaperimage path: '$(command -v plasma-apply-wallpaperimage || echo NOT_FOUND)'"
  if have plasma-apply-wallpaperimage; then
    msg "Apply wallpaper: $target"
    plasma-apply-wallpaperimage "$target" || true
  else
    msg "plasma-apply-wallpaperimage unavailable (skip wallpaper)"
  fi
}

read_config_value() {
  # Usage: read_config_value <mode> <key>
  local mode="$1" key="$2"
  if [ -f "$CONFIG_JSON" ]; then
    if have jq; then
      jq -r --arg m "$mode" --arg k "$key" '.[$m][$k] // empty' "$CONFIG_JSON" 2>/dev/null || echo ""
      return
    fi
    if have python3; then
      python3 - "$CONFIG_JSON" "$mode" "$key" 2>/dev/null <<'PYCODE' || true
import json, sys
path, mode, key = sys.argv[1], sys.argv[2], sys.argv[3]
with open(path, 'r', encoding='utf-8') as f:
    data = json.load(f)
val = data.get(mode, {}).get(key, "")
print(val if isinstance(val, str) else ("" if val is None else str(val)))
PYCODE
      return
    fi
    if have node; then
      node -e "const fs=require('fs');const d=JSON.parse(fs.readFileSync(process.argv[1],'utf8'));const v=((d[process.argv[2]]||{})[process.argv[3]]||'');process.stdout.write(String(v));" "$CONFIG_JSON" "$mode" "$key" 2>/dev/null || true
      return
    fi
  fi
  echo ""
}

die_config() {
  echo "[kde-theme-switch] Error: missing config.json or no JSON utility available (jq/python3/node). Aborting." >&2
  exit 99
}

assert_config() {
  if [ ! -f "$CONFIG_JSON" ]; then
    die_config
  fi
  if have jq || have python3 || have node; then
    return
  fi
  die_config
}

cfg_read() {
  # Usage: cfg_read <file> <group> <key>
  local file="$1" group="$2" key="$3"
  if ! have kreadconfig6; then return 1; fi
  kreadconfig6 --file "$file" --group "$group" --key "$key"
}

cfg_write() {
  # Usage: cfg_write <file> <group> <key> <value>
  local file="$1" group="$2" key="$3" value="$4"
  if ! have kwriteconfig6; then return 1; fi
  kwriteconfig6 --file "$file" --group "$group" --key "$key" "$value"
}

apply_kvantum_theme() {
  # Usage: apply_kvantum_theme <themeName>
  local theme="$1"
  if [ -z "$theme" ]; then
    return 0
  fi
  # Prefer kvantumctl (CLI, non-GUI)
  if have kvantumctl; then
    kvantumctl --set "$theme" >/dev/null 2>&1 || true
    return 0
  fi
  # Write config file directly (no GUI)
  mkdir -p "$HOME/.config/Kvantum"
  cat > "$HOME/.config/Kvantum/kvantum.kvconfig" <<EOF
[General]
theme=$theme
EOF
  # Attempt reload if kvantumctl is present
  if have kvantumctl; then
    kvantumctl --reload >/dev/null 2>&1 || true
  fi
}

sanitize_kdedefaults_kwinrc() {
  # Normalize ~/.config/kdedefaults/kwinrc
  local kdedef_dir="$HOME/.config/kdedefaults"
  local kdedef_file="$kdedef_dir/kwinrc"
  mkdir -p "$kdedef_dir"
  if [ -f "$kdedef_file" ]; then
    # Clean WhiteSur/Aurorae leftovers; do not set theme here (handled in apply_components)
    sed -i 's/__aurorae__svg__WhiteSur[^[:space:]]*/__aurorae__svg__pearOS/gI' "$kdedef_file" || true
  else
    cat > "$kdedef_file" <<EOF
[org.kde.kdecoration2]
Library=org.kde.kwin.aurorae
Theme=__aurorae__svg__pearOS
BorderSize=Normal
EOF
  fi
}

read_sddm_current() {
  # Read current SDDM theme from /etc/sddm.conf.d/*.conf or /etc/sddm.conf
  local files=()
  if [ -d /etc/sddm.conf.d ]; then
    while IFS= read -r -d '' f; do files+=("$f"); done < <(find /etc/sddm.conf.d -type f -name '*.conf' -print0 2>/dev/null | sort -z)
  fi
  if [ -f /etc/sddm.conf ]; then
    files+=("/etc/sddm.conf")
  fi
  local current=""
  for f in "${files[@]}"; do
    current="$(awk '
      BEGIN{insec=0}
      /^\[Theme\]/{insec=1;next}
      /^\[/{insec=0}
      insec && $0 ~ /^Current[[:space:]]*=/ {
        sub(/^[^=]*=/, "", $0);
        gsub(/^[[:space:]]+|[[:space:]]+$/, "", $0);
        print $0; exit
      }
    ' "$f" 2>/dev/null || true)"
    if [ -n "$current" ]; then
      echo "$current"
      return 0
    fi
  done
  return 1
}

write_sddm_current() {
  # Usage: write_sddm_current <themeName>
  # Create/update /etc/sddm.conf.d/10-theme.conf
  local theme="$1"
  [ -z "$theme" ] && return 0
  local tmpfile
  tmpfile="$(mktemp)"
  cat > "$tmpfile" <<EOF
[Theme]
Current=$theme
EOF
  if have sudo && sudo -n true 2>/dev/null; then
    sudo install -Dm0644 "$tmpfile" /etc/sddm.conf.d/10-theme.conf || true
  elif have pkexec; then
    pkexec install -Dm0644 "$tmpfile" /etc/sddm.conf.d/10-theme.conf || true
  else
    msg "Cannot write SDDM theme without privileges. Run as root:"
    msg "sudo install -Dm0644 \"$tmpfile\" /etc/sddm.conf.d/10-theme.conf"
  fi
  rm -f "$tmpfile"
}

list_dir_themes() {
  local label="$1"; shift
  local paths=("$@")
  echo "== $label =="
  local found=0
  for p in "${paths[@]}"; do
    if [ -d "$p" ]; then
      found=1
      echo "-- $p"
      find "$p" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' 2>/dev/null | sort -u
    fi
  done
  [ $found -eq 0 ] && echo "(nothing found)"
  echo
}

list_icon_themes() {
  echo "== Icon Themes =="
  local roots=( "$HOME/.icons" "$HOME/.local/share/icons" "/usr/share/icons" )
  local any=0
  for r in "${roots[@]}"; do
    if [ -d "$r" ]; then
      any=1
      echo "-- $r"
      find "$r" -mindepth 1 -maxdepth 1 -type d 2>/dev/null | while read -r d; do
        if [ -f "$d/index.theme" ]; then
          basename "$d"
        fi
      done | sort -u
    fi
  done
  [ $any -eq 0 ] && echo "(nothing found)"
  echo
}

list_aurorae() {
  echo "== Aurorae Window Decorations =="
  local paths=("$HOME/.local/share/aurorae/themes" "$HOME/.local/share/aurorae" "/usr/share/aurorae/themes" "/usr/share/aurorae")
  local any=0
  for p in "${paths[@]}"; do
    if [ -d "$p" ]; then
      any=1
      echo "-- $p"
      find "$p" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' 2>/dev/null | sort -u
    fi
  done
  echo "Note: Aurorae IDs for kwinrc look like __aurorae__svg__<Name>."
  [ $any -eq 0 ] && echo "(nothing found)"
  echo
}

list_all() {
  echo "Installed themes (KDE components):"
  echo

  echo "== Config (config.json) =="
  if [ -f "$CONFIG_JSON" ]; then
    if have jq; then
      echo "-- $(basename "$CONFIG_JSON")"
      jq '.' "$CONFIG_JSON" 2>/dev/null || cat "$CONFIG_JSON"
    else
      echo "-- $(basename "$CONFIG_JSON") (jq unavailable; printing raw)"
      cat "$CONFIG_JSON"
    fi
  else
    echo "(config.json not found next to this script)"
  fi
  # Show saved state (dark/light) if exists
  if [ -f "$(dirname "$0")/state" ]; then
    echo
    echo "== State =="
    echo "mode: $(cat "$(dirname "$0")/state" 2>/dev/null || echo unknown)"
  fi
  # Show saved accent if exists
  if [ -f "$(dirname "$0")/accent" ]; then
    echo
    echo "== Accent =="
    echo "accent: $(cat "$(dirname "$0")/accent" 2>/dev/null || echo unknown)"
  fi
  echo

  echo "== Application Style (Qt widget style) =="
  if have kreadconfig6; then
    current_style="$(cfg_read kdeglobals KDE widgetStyle 2>/dev/null || echo "")"
    if [ -n "$current_style" ]; then
      echo "current widgetStyle: $current_style"
    else
      echo "current widgetStyle: (unknown)"
    fi
  else
    echo "(kreadconfig6 unavailable)"
  fi
  echo

  echo "== Global (Look-and-Feel) =="
  if have lookandfeeltool; then
    lookandfeeltool -l || true
  else
    echo "(lookandfeeltool unavailable)"
  fi
  echo

  echo "== Accent presets (colors.json) =="
  if [ -d "$COLORS_DIR" ]; then
    # list all *.ini presets by filename (without extension)
    for f in "$COLORS_DIR"/*.ini; do
      [ -f "$f" ] || continue
      basename "${f%.ini}"
    done | sort -u
  else
    echo "(colors directory not found)"
  fi
  echo

  echo "== Color Schemes =="
  if have plasma-apply-colorscheme; then
    plasma-apply-colorscheme -l || true
  else
    list_dir_themes "Color Schemes (fallback)" "$HOME/.local/share/color-schemes" "/usr/share/color-schemes"
  fi

  echo "== Plasma Desktop Themes =="
  if have plasma-apply-desktoptheme; then
    plasma-apply-desktoptheme -l || true
  else
    list_dir_themes "Plasma Desktop Themes (fallback)" "$HOME/.local/share/plasma/desktoptheme" "/usr/share/plasma/desktoptheme"
  fi

  echo "== Cursor Themes =="
  if have plasma-apply-cursortheme; then
    plasma-apply-cursortheme -l || true
  else
    echo "(plasma-apply-cursortheme unavailable; listing from directories)"
    list_icon_themes
  fi

  list_icon_themes
  list_aurorae

  echo "== Kvantum Themes =="
    ls /usr/share/Kvantum/
  echo "== KWin Decorations (current) =="
  if have kreadconfig6; then
    echo -n "Library: "; cfg_read kwinrc org.kde.kdecoration2 Library 2>/dev/null || echo
    echo -n "Theme: "; cfg_read kwinrc org.kde.kdecoration2 Theme 2>/dev/null || echo
  fi

  echo "== SDDM Themes =="
  if [ -d /usr/share/sddm/themes ]; then
    echo "-- /usr/share/sddm/themes"
    find /usr/share/sddm/themes -mindepth 1 -maxdepth 1 -type d -printf '%f\n' 2>/dev/null | sort -u
  else
    echo "(no /usr/share/sddm/themes found)"
  fi
  local sddm_cur
  sddm_cur="$(read_sddm_current 2>/dev/null || echo "")"
  if [ -n "$sddm_cur" ]; then
    echo "Current SDDM theme: $sddm_cur"
  else
    echo "Current SDDM theme: (unknown)"
  fi
}

apply_components() {
  local mode="$1"
  assert_config
  local COLOR PTHM ITHM CTHM KVT GTK SDDM AUR_THEME APPSTYLE
  COLOR="$(read_config_value "$mode" colorScheme)"
  PTHM="$(read_config_value "$mode" plasmaTheme)"
  ITHM="$(read_config_value "$mode" iconTheme)"
  CTHM="$(read_config_value "$mode" cursorTheme)"
  KVT="$(read_config_value "$mode" kvantumTheme)"
  GTK="$(read_config_value "$mode" gtkTheme)"
  SDDM="$(read_config_value "$mode" sddmTheme)"
  AUR_THEME="$(read_config_value "$mode" auroraeTheme)"
  APPSTYLE="$(read_config_value "$mode" appStyle)"
  for k in COLOR PTHM ITHM CTHM KVT GTK SDDM AUR_THEME APPSTYLE; do
    if [ -z "${!k:-}" ]; then
      echo "[kde-theme-switch] Error: missing key in config.json for mode '$mode': $k" >&2
      exit 99
    fi
  done

  msg "Apply color scheme: $COLOR"
  if have plasma-apply-colorscheme; then
    plasma-apply-colorscheme "$COLOR" || true
  else
    msg "plasma-apply-colorscheme unavailable (skip)"
  fi

  # Adjust wallpaper only if current wallpaper is one of pearOS defaults
  set_wallpaper_if_pearos "$mode"

  msg "Apply Plasma desktop theme: $PTHM"
  if have plasma-apply-desktoptheme; then
    plasma-apply-desktoptheme "$PTHM" || true
  else
    msg "plasma-apply-desktoptheme unavailable (skip)"
  fi

  msg "Set icon theme: $ITHM"
  if have kwriteconfig6; then
    cfg_write kdeglobals Icons Theme "$ITHM" || true
  else
    msg "kwriteconfig6 unavailable (skip Icons)"
  fi

  msg "Apply cursor theme: $CTHM"
  if have plasma-apply-cursortheme; then
    plasma-apply-cursortheme "$CTHM" || true
  else
    msg "plasma-apply-cursortheme unavailable (skip)"
  fi

  # Application Style (Qt widget style)
  # APPSTYLE may come from config.json
  msg "Set Application Style (Qt widget style): $APPSTYLE"
  if have kwriteconfig6; then
    # Ensure plugin is 'kvantum' regardless of Kvantum theme
    cfg_write kdeglobals KDE widgetStyle "kvantum" || true
  else
    msg "kwriteconfig6 unavailable (skip Application Style)"
  fi

  msg "Normalize kdedefaults/kwinrc and apply Aurorae for Window Decorations"
  sanitize_kdedefaults_kwinrc
  # Write both uppercase and lowercase variants for compatibility
  if have kwriteconfig6; then
    cfg_write kwinrc org.kde.kdecoration2 Library "$KWIN_DECORATION_PLUGIN" || true
    cfg_write kwinrc org.kde.kdecoration2 Theme "$AUR_THEME" || true
    cfg_write kwinrc org.kde.kdecoration2 library "$KWIN_DECORATION_PLUGIN" || true
    cfg_write kwinrc org.kde.kdecoration2 theme "$AUR_THEME" || true
    # macOS-like layout on the left
    cfg_write kwinrc org.kde.kdecoration2 ButtonsOnLeft "XIA" || true
    cfg_write kwinrc org.kde.kdecoration2 ButtonsOnRight "" || true
  fi
  # Update kdedefaults to current theme to avoid overrides at login
  local kdedef_file="$HOME/.config/kdedefaults/kwinrc"
  if [ -f "$kdedef_file" ]; then
    sed -i 's/^[[:space:]]*[Ll]ibrary[[:space:]]*=.*/Library=org.kde.kwin.aurorae/I' "$kdedef_file" || true
    sed -i "s/^[[:space:]]*[Tt]heme[[:space:]]*=.*/Theme=${AUR_THEME//\//\\/}/I" "$kdedef_file" || true
  fi
  # Confirmation
  if have kreadconfig6; then
    local set_lib set_theme
    set_lib="$(cfg_read kwinrc org.kde.kdecoration2 Library 2>/dev/null || echo "")"
    set_theme="$(cfg_read kwinrc org.kde.kdecoration2 Theme 2>/dev/null || echo "")"
    msg "KWin decoration set: Library='$set_lib' Theme='${set_theme}'"
  fi

  if [ -n "${KVT:-}" ]; then
    msg "Apply Kvantum: $KVT"
    apply_kvantum_theme "$KVT"
  else
    msg "Kvantum not configured (skip)"
  fi

  msg "Set GTK theme (optional): $GTK"
  if have gsettings; then
    gsettings set org.gnome.desktop.interface gtk-theme "$GTK" || true
    gsettings set org.gnome.desktop.interface icon-theme "$ITHM" || true
    gsettings set org.gnome.desktop.interface cursor-theme "$CTHM" || true
  else
    msg "gsettings unavailable (skip GTK)"
  fi

  # Fallback for GTK3/GTK4 apps that ignore gsettings
  write_gtk_ini() {
    local target_dir="$1"
    local theme="$2"
    local icon="$3"
    local cursor="$4"
    local prefer_dark="$5"
    mkdir -p "$target_dir"
    cat > "$target_dir/settings.ini" <<EOF
[Settings]
gtk-theme-name=$theme
gtk-icon-theme-name=$icon
gtk-cursor-theme-name=$cursor
gtk-application-prefer-dark-theme=$prefer_dark
EOF
  }
  local prefer_dark_val
  if [ "$mode" = "dark" ]; then
    prefer_dark_val=1
  else
    prefer_dark_val=0
  fi
  msg "Write fallback GTK settings.ini (GTK3/GTK4)"
  write_gtk_ini "$HOME/.config/gtk-3.0" "$GTK" "$ITHM" "$CTHM" "$prefer_dark_val"
  write_gtk_ini "$HOME/.config/gtk-4.0" "$GTK" "$ITHM" "$CTHM" "$prefer_dark_val"

  configure_gtk_kvantum() {
    local kv_theme="$1"
    local gtk_theme="$2"
    local theme_dir="$HOME/.themes/${gtk_theme}/gtk-4.0"
    # Kvantum
    mkdir -p "$HOME/.config/Kvantum"
    if [ -f "$HOME/.config/Kvantum/kvantum.kvconfig" ]; then
      sed -i "s/^theme=.*/theme=${kv_theme}/" "$HOME/.config/Kvantum/kvantum.kvconfig" || true
    else
      printf "[General]\ntheme=%s\n" "$kv_theme" > "$HOME/.config/Kvantum/kvantum.kvconfig"
    fi
    # GTK2
    if [ -f "$HOME/.gtkrc-2.0" ]; then
      sed -i "s/^gtk-theme-name=.*/gtk-theme-name=\"${gtk_theme}\"/" "$HOME/.gtkrc-2.0" || true
    else
      echo "gtk-theme-name=\"${gtk_theme}\"" > "$HOME/.gtkrc-2.0"
    fi
    # GTK3
    mkdir -p "$HOME/.config/gtk-3.0"
    if [ -f "$HOME/.config/gtk-3.0/settings.ini" ]; then
      if grep -q "^gtk-theme-name=" "$HOME/.config/gtk-3.0/settings.ini" 2>/dev/null; then
        sed -i "s/^gtk-theme-name=.*/gtk-theme-name=${gtk_theme}/" "$HOME/.config/gtk-3.0/settings.ini" || true
      else
        # ensure [Settings] exists
        if ! grep -q "^\[Settings\]" "$HOME/.config/gtk-3.0/settings.ini" 2>/dev/null; then
          printf "[Settings]\n" >> "$HOME/.config/gtk-3.0/settings.ini"
        fi
        printf "gtk-theme-name=%s\n" "$gtk_theme" >> "$HOME/.config/gtk-3.0/settings.ini"
      fi
    else
      printf "[Settings]\ngtk-theme-name=%s\n" "$gtk_theme" > "$HOME/.config/gtk-3.0/settings.ini"
    fi
    # GTK4
    mkdir -p "$HOME/.config/gtk-4.0"
    if [ -f "$HOME/.config/gtk-4.0/settings.ini" ]; then
      if grep -q "^gtk-theme-name=" "$HOME/.config/gtk-4.0/settings.ini" 2>/dev/null; then
        sed -i "s/^gtk-theme-name=.*/gtk-theme-name=${gtk_theme}/" "$HOME/.config/gtk-4.0/settings.ini" || true
      else
        if ! grep -q "^\[Settings\]" "$HOME/.config/gtk-4.0/settings.ini" 2>/dev/null; then
          printf "[Settings]\n" >> "$HOME/.config/gtk-4.0/settings.ini"
        fi
        printf "gtk-theme-name=%s\n" "$gtk_theme" >> "$HOME/.config/gtk-4.0/settings.ini"
      fi
    else
      printf "[Settings]\ngtk-theme-name=%s\n" "$gtk_theme" > "$HOME/.config/gtk-4.0/settings.ini"
    fi
    # GTK4 assets symlink
    rm -rf "$HOME/.config/gtk-4.0/assets" || true
    if [ -d "$theme_dir" ]; then
      ln -sf "$theme_dir/assets" "$HOME/.config/gtk-4.0/assets"
      ln -sf "$theme_dir/gtk.css" "$HOME/.config/gtk-4.0/gtk.css"
      ln -sf "$theme_dir/gtk-dark.css" "$HOME/.config/gtk-4.0/gtk-dark.css"
    fi
  }
  configure_gtk_kvantum "$KVT" "$GTK"

  local libadwaita_tool
  libadwaita_tool="$(dirname "$0")/libadwaita"
  if [ -x "$libadwaita_tool" ]; then
    if [ "$mode" = "dark" ]; then
      "$libadwaita_tool" --dark || true
    else
      "$libadwaita_tool" --light || true
    fi
  fi

  if [ -n "${SDDM:-}" ]; then
    msg "Set SDDM theme: $SDDM"
    local tmpfile
    tmpfile="$(mktemp)"
    cat > "$tmpfile" <<EOF
[Theme]
Current=$SDDM
EOF
    if have sudo && sudo -n true 2>/dev/null; then
      sudo install -Dm0644 "$tmpfile" /etc/sddm.conf.d/10-theme.conf || true
    elif have pkexec; then
      pkexec install -Dm0644 "$tmpfile" /etc/sddm.conf.d/10-theme.conf || true
    else
    msg "Cannot write SDDM without privileges. Run manually:"
      echo "  sudo install -Dm0644 \"$tmpfile\" /etc/sddm.conf.d/10-theme.conf"
    fi
    rm -f "$tmpfile"
    local sddm_after
    sddm_after="$(read_sddm_current 2>/dev/null || echo "")"
    if [ "$sddm_after" = "$SDDM" ]; then
      msg "SDDM confirmat: $sddm_after"
    else
      msg "WARNING: SDDM is still '$sddm_after' (likely missing privileges or overridden in other files under /etc/sddm.conf.d)"
    fi
  else
    msg "SDDM not configured (skip)"
  fi

  # Reconfigure KWin so changes to decorations take effect
  if have qdbus; then
    qdbus org.kde.KWin /KWin reconfigure >/dev/null 2>&1 || true
  elif have qdbus6; then
    qdbus6 org.kde.KWin /KWin reconfigure >/dev/null 2>&1 || true
  fi
  if [ "${XDG_SESSION_TYPE:-}" = "x11" ] && have kwin_x11; then
    (kwin_x11 --replace >/dev/null 2>&1 &) || true
  fi

  msg "Done ($mode). Some applications may require restart."

  STATE_FILE="$(dirname "$0")/state"
  printf "%s\n" "$mode" > "$STATE_FILE" || true
}

detect_and_toggle() {
  local current
  if have kreadconfig6; then
    current="$(cfg_read kdeglobals General ColorScheme 2>/dev/null || echo "")"
  else
    current=""
  fi
  if echo "$current" | grep -qi "dark"; then
    apply_components light
  else
    apply_components dark
  fi
}

usage() {
  cat <<EOF
Usage:
  $(basename "$0")                 Toggle theme based on ./state (light <-> dark)
  $(basename "$0") --dark          Apply dark settings (all components)
  $(basename "$0") --light         Apply light settings (all components)
  $(basename "$0") --toggle        Toggle based on current ColorScheme (Dark<->Light)
  $(basename "$0") --list          List installed themes and current configuration
  $(basename "$0") --color-list    List available accent presets from colors/*.ini
  $(basename "$0") --accent NAME   Apply accent preset NAME (from colors/*.ini)

Note:
  - Config is loaded from ./config.json, or /usr/share/extras/pearos-themesw/config.json.
  - Accent presets are read from ./colors/*.ini, or /usr/share/extras/pearos-themesw/colors/*.ini.
  - After --accent, KWin and plasmashell are reloaded so changes take effect immediately.
EOF
}

main() {
  if [ $# -lt 1 ]; then
    local STATE_FILE="$(dirname "$0")/state"
    local next="dark"
    if [ -f "$STATE_FILE" ]; then
      case "$(tr '[:upper:]' '[:lower:]' < "$STATE_FILE" | tr -d '\n\r\t ')" in
        dark) next="light" ;;
        light) next="dark" ;;
        *) next="dark" ;;
      esac
    fi
    apply_components "$next"
    exit 0
  fi
  case "${1:-}" in
    --dark) apply_components dark ;;
    --light) apply_components light ;;
    --toggle) detect_and_toggle ;;
    --color-list)
      echo "Available accent presets:"
      if [ -d "$COLORS_DIR" ]; then
        for f in "$COLORS_DIR"/*.ini; do
          [ -f "$f" ] || continue
          basename "${f%.ini}"
        done | sort -u
      else
        echo "[kde-theme-switch] Error: colors directory not found ($COLORS_DIR)." >&2
        exit 99
      fi
      ;;
    --accent)
      shift || true
      if [ -z "${1:-}" ]; then
        echo "[kde-theme-switch] Usage: $(basename "$0") --accent <preset>" >&2
        exit 1
      fi
      preset="$1"
      if ! have python3; then
        echo "[kde-theme-switch] Error: python3 required for --accent." >&2
        exit 99
      fi
      python3 - "$COLORS_DIR" "$preset" <<'PY'
import sys, os, subprocess, configparser

colors_dir, preset = sys.argv[1], sys.argv[2]
ini_path = os.path.join(colors_dir, preset + ".ini")
if not os.path.isfile(ini_path):
    print(f"[kde-theme-switch] Error: preset '{preset}' not found as {ini_path}.", file=sys.stderr)
    sys.exit(99)

cp = configparser.ConfigParser(interpolation=None)
cp.optionxform = str  # preserve case of keys
cp.read(ini_path, encoding="utf-8")

def write_key(section, key, value):
    try:
        subprocess.run(
            ["kwriteconfig6", "--file", "kdeglobals", "--group", section, "--key", key, value],
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except Exception:
        pass

for section in cp.sections():
    # We care mainly about Colors:* and General sections for accent
    if not (section.startswith("Colors:") or section == "General"):
        continue
    for key, value in cp[section].items():
        write_key(section, key, value)

sys.exit(0)
PY
      # Save selected accent to file (similar to state for dark/light)
      ACCENT_FILE="$(dirname "$0")/accent"
      printf "%s\n" "$preset" > "$ACCENT_FILE" || true
      msg "Saved accent: $preset"
      # Re-apply ColorScheme according to current state (to keep scheme + accent in sync)
      if [ -f "$(dirname "$0")/state" ]; then
        state_mode="$(tr '[:upper:]' '[:lower:]' < "$(dirname "$0")/state" | tr -d '\n\r\t ' 2>/dev/null || echo "")"
        case "$state_mode" in
          dark|light)
            if [ -f "$CONFIG_JSON" ]; then
              # do not force assert_config here; fall back silently if JSON tools missing
              cs_val="$(read_config_value "$state_mode" colorScheme)"
              if [ -n "$cs_val" ] && have plasma-apply-colorscheme; then
                msg "Re-apply color scheme after accent: $cs_val"
                plasma-apply-colorscheme "$cs_val" || true
              fi
            fi
            ;;
        esac
      fi
      # reload KWin to apply changes
      if have qdbus6; then
        qdbus6 org.kde.KWin /KWin reconfigure >/dev/null 2>&1 || true
      elif have qdbus; then
        qdbus org.kde.KWin /KWin reconfigure >/dev/null 2>&1 || true
      fi
      # restart plasmashell so accent + scheme are fully applied
      if have kquitapp6; then
        kquitapp6 plasmashell >/dev/null 2>&1 || true
        (plasmashell >/dev/null 2>&1 &) || true
      elif have kquitapp5; then
        kquitapp5 plasmashell >/dev/null 2>&1 || true
        (plasmashell >/dev/null 2>&1 &) || true
      fi
      ;;
    --list) list_all ;;
    -h|--help) usage ;;
    *) usage; exit 1 ;;
  esac
}

main "$@"