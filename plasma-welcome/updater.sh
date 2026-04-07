#!/usr/bin/bash
pkexec pacman -Syy --overwrite='*' --noconfirm \
  filesystem pearos-notes pearos-appstore pearos-calendar pearos-contacts pearos-notch \
  pearos-todo pearos-calculator pearos-liquidgel pearos-window-borders pearos-settings \
  system-overview kmail gnome-maps
