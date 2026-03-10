#!/bin/bash
# Script de build pentru pearos-calculator
# Creează tarball-ul sursă și compilează pachetul cu makepkg

set -e
cd "$(dirname "$0")"

# Citește versiunea din PKGBUILD
pkgname=$(grep -E '^pkgname=' PKGBUILD | cut -d= -f2 | tr -d "'\"")
pkgver=$(grep -E '^pkgver=' PKGBUILD | cut -d= -f2 | tr -d "'\"")

tarball="${pkgname}-${pkgver}.tar.gz"
srcdir="${pkgname}-${pkgver}"

echo "==> Creând tarball sursă: ${tarball}"

# Curăță un eventual build anterior
rm -rf "${srcdir}" "${tarball}"

# Copiază sursele într-un director cu numele așteptat de PKGBUILD
mkdir -p "${srcdir}"
tar --exclude='.git' \
    --exclude='build' \
    --exclude='pkg' \
    --exclude='src' \
    --exclude='*.tar.gz' \
    --exclude="${srcdir}" \
    -cf - . | tar -C "${srcdir}" -xf -

# Creează arhiva
tar -czf "${tarball}" "${srcdir}"
rm -rf "${srcdir}"

echo "==> Construind pachetul cu makepkg..."
makepkg -sf --noconfirm

echo "==> Gata. Pachetul a fost construit."
