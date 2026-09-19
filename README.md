# MECHREVO XINGYAO P916F Linux enablement

Experimental, hardware-scoped Linux support for the MECHREVO XINGYAO Series-P916F-HPT-R.

## Current findings

- DMI: `MECHREVO / XINGYAO Series / XINGYAO Series-P916F-HPT-R`
- WMI method GUID: `ABBC0F5B-8EA1-11D1-A000-C90629100000`, object `AA`, 2 instances
- WMI event GUID: `ABBC0F5C-8EA1-11D1-A000-C90629100000`, notify ID `A0`
- The GUIDs collide with Huawei laptops, but the firmware ABI is different. The P916F `WMAA` method accepts one argument and only returns the current `WMEN` event value, while the legacy Huawei driver invokes it as a three-argument control method. This produces `Excess arguments` and `Unexpected obj type` errors.
- AMD PMF already provides standard `low-power`, `balanced`, and `performance` platform profiles. The platform driver must not register a duplicate profile provider.
- The ALC256 codec subsystem `1d05:e005` needs pin `0x1b` identified as a second internal speaker.

### Confirmed firmware event ABI

The DSDT assigns these values to `WMEN` before issuing WMI notify `0xA0`:

| Event | Firmware meaning |
|---|---|
| `0x20`, `0x21`, `0x22` | keyboard-backlight level 0, 1, 2 |
| `0x30`, `0x31` | touchpad off, on |
| `0x41`, `0x42` | balanced, performance profile |
| `0xA0`, `0xA1` | not identified; logged but not mapped |

The EC field `FTVL` contains the firmware thermal mode. `THMM()` applies the matching AMD DPTC limits through `ALIB`: balanced uses 15/30/25 W values and performance uses 28/45/35 W values. AMD PMF already exposes these firmware choices through the standard platform-profile interface.

No fan tachometer/control method or battery charge-limit field exists in the captured ACPI tables. Standard `hwmon` fan control and charge thresholds therefore cannot be implemented safely from this firmware revision. Existing generic `acpitz`, `k10temp`, `amdgpu`, ACPI battery and AMD PMF interfaces remain authoritative.

## Driver status

`mechrevo-p916f-wmi.c` is deliberately read-only with respect to firmware. It:

- refuses to probe on any machine except the exact P916F DMI match;
- binds only the WMI event device;
- exposes confirmed hotkey events through the standard Linux input subsystem;
- logs unknown event codes so they can be mapped safely later;
- performs no EC or WMI writes.

Fan, charge-limit and LED controls will only be added after the ACPI method contract has been decoded. Guessing EC writes is unsafe.

The kernel-ready draft is in
[`patches/0001-platform-x86-Add-MECHREVO-P916F-WMI-hotkey-driver.patch`](patches/0001-platform-x86-Add-MECHREVO-P916F-WMI-hotkey-driver.patch).
It also prevents `huawei-wmi` from probing this exact DMI model, because that
driver's three-argument control ABI is incompatible with the P916F firmware.
The patch passes `checkpatch.pl --strict` with no errors; its sole advisory is
the generic question about adding a new file to `MAINTAINERS`, which is already
covered by the existing X86 PLATFORM DRIVERS entry.

## Build

```sh
make
```

Loading requires root and the conflicting `huawei_wmi` module must first be unloaded. Do not load the experimental module until the ACPI capture has been reviewed.

The captured firmware has now been reviewed and the module performs no firmware writes. To run a temporary test for the current boot:

```sh
sudo ./test-driver.sh
```

Press the touchpad, keyboard-backlight and Fn+X profile keys, then inspect messages with:

```sh
sudo dmesg | grep -E 'mechrevo|hotkey'
```

Rebooting restores the distribution driver state; this test does not install the module persistently.

## Persistent installation

The installer first verifies the exact supported DMI identity. It then installs
the module through DKMS so it is rebuilt for kernel updates, blacklists the
incompatible `huawei_wmi` driver on this machine, updates the initramfs, and
loads the new driver:

```sh
sudo ./install.sh
```

Reboot once and check the persistent state:

```sh
sudo ./verify-install.sh
sudo ./test-events.py
```

To remove the DKMS module and restore the distribution driver policy:

```sh
sudo ./uninstall.sh
```

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
