#!/bin/sh
set -eu

package=mechrevo-p916f-piix4-quirk
version=0.1.0
source_dir=/usr/src/$package-$version

if [ "$(id -u)" -ne 0 ]; then
	echo "Run with sudo: sudo $0" >&2
	exit 1
fi

[ "$(cat /sys/class/dmi/id/sys_vendor)" = "MECHREVO" ]
[ "$(cat /sys/class/dmi/id/product_name)" = "XINGYAO Series" ]
[ "$(cat /sys/class/dmi/id/board_name)" = "XINGYAO Series-P916F-HPT-R" ]

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

if dkms status -m "$package" -v "$version" 2>/dev/null | grep -q .; then
	dkms remove -m "$package" -v "$version" --all
fi

install -d "$source_dir"
install -m 0644 "$script_dir/i2c-piix4.c" "$source_dir/"
install -m 0644 "$script_dir/i2c-piix4.h" "$source_dir/"
install -m 0644 "$script_dir/Makefile" "$source_dir/"
install -m 0644 "$script_dir/dkms.conf" "$source_dir/"

dkms add -m "$package" -v "$version"
dkms build -m "$package" -v "$version"
dkms install -m "$package" -v "$version"
update-initramfs -u

modprobe -r spd5118 2>/dev/null || true
modprobe -r i2c_piix4
modprobe i2c_piix4

echo "Installed P916F PIIX4 SPD-probe quirk."
