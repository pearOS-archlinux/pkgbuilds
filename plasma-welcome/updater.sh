#!/usr/bin/bash

pkexec pacman -Syy --overwrite='*' --noconfirm \
  filesystem pearos-notes pearos-appstore pearos-calendar pearos-contacts pearos-notch \
  pearos-todo pearos-calculator pearos-liquidgel pearos-window-borders pearos-settings \
  system-overview kmail gnome-maps

if [ $? -ne 0 ]; then
  exit 1
fi

SKEL_CONFIG="/etc/skel/.config"
USER_CONFIG="${HOME}/.config"

PLASMA_FILES=(
  "plasma-org.kde.plasma.desktop-appletsrc"
  "plasmarc"
  "plasmashellrc"
  "kdeglobals"
)

GTK_FILES=(
  "gtk-2.0"
  "gtk-3.0"
  "gtk-4.0"
  "gtkrc"
  "gtkrc-2.0"
  "Trolltech.conf"
)

ALL_OK=true

for f in "${PLASMA_FILES[@]}" "${GTK_FILES[@]}"; do
  src="${SKEL_CONFIG}/${f}"
  dst="${USER_CONFIG}/${f}"

  if [ ! -e "${src}" ]; then
    continue
  fi

  if [ -d "${src}" ]; then
    rm -rf "${dst}"
    cp -r "${src}" "${dst}"
  else
    cp "${src}" "${dst}"
  fi

  if [ $? -ne 0 ] || [ ! -e "${dst}" ]; then
    ALL_OK=false
  fi
done

if [ "${ALL_OK}" = false ]; then
  exit 2
fi
