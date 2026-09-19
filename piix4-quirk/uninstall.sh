#!/bin/sh
set -eu

package=mechrevo-p916f-piix4-quirk
version=0.1.0

if [ "$(id -u)" -ne 0 ]; then
	echo "Run with sudo: sudo $0" >&2
	exit 1
fi

modprobe -r spd5118 2>/dev/null || true
modprobe -r i2c_piix4 2>/dev/null || true
dkms remove -m "$package" -v "$version" --all 2>/dev/null || true
update-initramfs -u
modprobe i2c_piix4

echo "Restored the distribution i2c_piix4 module."
