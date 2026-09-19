#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
	echo "Run with sudo: sudo $0" >&2
	exit 1
fi

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

grep -qx 'MECHREVO' /sys/class/dmi/id/sys_vendor
grep -qx 'XINGYAO Series' /sys/class/dmi/id/product_name
grep -qx 'XINGYAO Series-P916F-HPT-R' /sys/class/dmi/id/board_name

modprobe -r huawei_wmi 2>/dev/null || true
rmmod mechrevo_p916f_wmi 2>/dev/null || true
modprobe wmi
modprobe sparse-keymap
insmod "$script_dir/mechrevo-p916f-wmi.ko"

echo "Loaded mechrevo_p916f_wmi. Press the touchpad, keyboard-light and Fn+X keys."
echo "Then inspect events with: sudo dmesg | grep -E 'mechrevo|hotkey'"
