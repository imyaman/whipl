#!/bin/bash
set -e

PREFIX="${PREFIX:-/usr/local}"
BINDIR="$PREFIX/bin"
LIBDIR="$PREFIX/lib/whipl"

echo "Uninstalling whipl from $PREFIX"

# Check if we need sudo
SUDO=""
if [ ! -w "$PREFIX" ]; then
    SUDO="sudo"
fi

# Remove binary
if [ -f "$BINDIR/whipl" ]; then
    echo "Removing $BINDIR/whipl"
    $SUDO rm -f "$BINDIR/whipl"
fi

# Remove library directory
if [ -d "$LIBDIR" ]; then
    echo "Removing $LIBDIR"
    $SUDO rm -rf "$LIBDIR"
fi

echo
echo "Uninstallation complete!"
