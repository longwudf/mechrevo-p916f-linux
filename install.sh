#!/bin/sh
set -eu

package=mechrevo-p916f-wmi
version=0.1.0
source_dir=/usr/src/$package-$version

if [ "$(id -u)" -ne 0 ]; then
	echo "Run with sudo: sudo $0" >&2
	exit 1
fi

check_dmi()
{
	[ "$(cat /sys/class/dmi/id/sys_vendor)" = "MECHREVO" ] &&
	[ "$(cat /sys/class/dmi/id/product_name)" = "XINGYAO Series" ] &&
	[ "$(cat /sys/class/dmi/id/board_name)" = "XINGYAO Series-P916F-HPT-R" ]
}

if ! check_dmi; then
	echo "Refusing to install: this is not the supported MECHREVO P916F." >&2
	exit 1
fi

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

if ! command -v dkms >/dev/null 2>&1; then
	if command -v apt-get >/dev/null 2>&1; then
		apt-get update
		apt-get install -y dkms build-essential "linux-headers-$(uname -r)"
	else
		echo "Install DKMS, a compiler, make, and the running kernel headers first." >&2
		exit 1
	fi
fi

if dkms status -m "$package" -v "$version" 2>/dev/null | grep -q .; then
	dkms remove -m "$package" -v "$version" --all
fi

rm -rf "$source_dir"
install -d "$source_dir"
install -m 0644 "$script_dir/mechrevo-p916f-wmi.c" "$source_dir/"
install -m 0644 "$script_dir/Makefile" "$source_dir/"
install -m 0644 "$script_dir/dkms.conf" "$source_dir/"

dkms add -m "$package" -v "$version"
dkms build -m "$package" -v "$version"
dkms install -m "$package" -v "$version"

cat > /etc/modprobe.d/mechrevo-p916f-wmi.conf <<'EOF'
# The P916F reuses Huawei WMI GUIDs with an incompatible one-argument ABI.
blacklist huawei_wmi
EOF

cat > /etc/modules-load.d/mechrevo-p916f-wmi.conf <<'EOF'
mechrevo_p916f_wmi
EOF

if command -v update-initramfs >/dev/null 2>&1; then
	update-initramfs -u
fi

modprobe -r huawei_wmi 2>/dev/null || true
modprobe -r mechrevo_p916f_wmi 2>/dev/null || true
modprobe mechrevo_p916f_wmi

echo "Installed and loaded $package $version through DKMS."
echo "Reboot once, then verify with: sudo ./verify-install.sh"
