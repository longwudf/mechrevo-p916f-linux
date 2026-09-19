# MECHREVO XINGYAO P916F Linux enablement

Experimental, hardware-scoped Linux support for the MECHREVO XINGYAO Series-P916F-HPT-R.

## Current findings

- DMI: `MECHREVO / XINGYAO Series / XINGYAO Series-P916F-HPT-R`
- WMI method GUID: `ABBC0F5B-8EA1-11D1-A000-C90629100000`, object `AA`, 2 instances
- WMI event GUID: `ABBC0F5C-8EA1-11D1-A000-C90629100000`, notify ID `A0`
- The GUIDs collide with Huawei laptops, but the firmware ABI is different. The P916F `WMAA` ACPI method accepts one argument while the legacy Huawei driver invokes the standard three-argument WMI wrapper. This produces `Excess arguments` and `Unexpected obj type` errors.
- AMD PMF already provides standard `low-power`, `balanced`, and `performance` platform profiles. The platform driver must not register a duplicate profile provider.
- The ALC256 codec subsystem `1d05:e005` needs pin `0x1b` identified as a second internal speaker.

## Driver status

`mechrevo-p916f-wmi.c` is deliberately read-only with respect to firmware. It:

- refuses to probe on any machine except the exact P916F DMI match;
- binds only the WMI event device;
- exposes confirmed hotkey events through the standard Linux input subsystem;
- logs unknown event codes so they can be mapped safely later;
- performs no EC or WMI writes.

Fan, charge-limit and LED controls will only be added after the ACPI method contract has been decoded. Guessing EC writes is unsafe.

## Build

```sh
make
```

Loading requires root and the conflicting `huawei_wmi` module must first be unloaded. Do not load the experimental module until the ACPI capture has been reviewed.

## Firmware capture

Install `acpica-tools`, then run:

```sh
sudo ./collect-firmware.sh
```

The archive stays local and may contain firmware identifiers. Review it before sharing publicly.

## Audio patch

The proposed upstream codec quirk remains in the companion repository:

<https://github.com/longwudf/alc256-e005-linux-fix>

## License

GPL-2.0-only, matching the Linux kernel.

