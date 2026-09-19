#!/bin/sh
set -eu

package=mechrevo-p916f-wmi
version=0.1.0
source_dir=/usr/src/$package-$version

if [ "$(id -u)" -ne 0 ]; then
	echo "Run with sudo: sudo $0" >&2
	exit 1
fi

modprobe -r mechrevo_p916f_wmi 2>/dev/null || true
rm -f /etc/modules-load.d/mechrevo-p916f-wmi.conf
rm -f /etc/modprobe.d/mechrevo-p916f-wmi.conf

if command -v dkms >/dev/null 2>&1; then
	dkms remove -m "$package" -v "$version" --all 2>/dev/null || true
fi
rm -rf "$source_dir"

if command -v update-initramfs >/dev/null 2>&1; then
	update-initramfs -u
fi

modprobe huawei_wmi 2>/dev/null || true
echo "Removed $package $version and restored the distribution driver policy."
