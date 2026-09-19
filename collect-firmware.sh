#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
	echo "Run with sudo: sudo $0" >&2
	exit 1
fi

command -v acpidump >/dev/null || {
	echo "Install acpica-tools first" >&2
	exit 1
}
command -v iasl >/dev/null || {
	echo "iasl is missing (normally supplied by acpica-tools)" >&2
	exit 1
}

out="p916f-firmware-capture"
rm -rf "$out"
mkdir -p "$out/acpi" "$out/wmi"

acpidump -b -z -o "$out/acpi/acpidump.dat"
acpixtract -a "$out/acpi/acpidump.dat"
for table in dsdt.dat ssdt*.dat; do
	[ -f "$table" ] || continue
	mv "$table" "$out/acpi/"
done
(
	cd "$out/acpi"
	iasl -e ssdt*.dat -d dsdt.dat 2>iasl-errors.txt || true
)

for dev in /sys/bus/wmi/devices/*; do
	name=${dev##*/}
	mkdir -p "$out/wmi/$name"
	for attr in guid instance_count object_id notify_id expensive setable modalias uevent; do
		[ -r "$dev/$attr" ] && cp "$dev/$attr" "$out/wmi/$name/$attr"
	done
	[ -r "$dev/bmof" ] && cp "$dev/bmof" "$out/wmi/$name/bmof"
done

cp /sys/class/dmi/id/{sys_vendor,product_name,product_version,board_vendor,board_name,board_version,bios_vendor,bios_version,bios_date} "$out/" 2>/dev/null || true
dmesg > "$out/dmesg.txt"
tar -czf "$out.tar.gz" "$out"
echo "Created $out.tar.gz"

