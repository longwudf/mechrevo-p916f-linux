# P916F custom kernel

This directory documents the board-specific changes used by the
`7.0.14-p916f1` test kernel.  The build starts from Ubuntu Noble source package
`linux-hwe-7.0 7.0.0-31.31~24.04.1` and keeps the stock kernel installed as a
recovery option.

## Fixes

1. `AMDI0010:01` uses the AMD PSP arbitration semaphore on the exact P916F DMI
   match.  This prevents firmware and Linux from concurrently driving the
   touchpad I2C bus.
2. The duplicate `\\_SB.PCI0.GPP6.WLAN._DSM` supplied by the later SSDT is
   accepted only on the exact P916F DMI match.  This activates the firmware's
   WLAN D3cold policy instead of rejecting the SSDT with `AE_ALREADY_EXISTS`.
3. The unusable primary PIIX4 SMBus port is not probed for SPD write-enable on
   this board.  DIMM SPD devices on the auxiliary port remain available.

The implementation is intentionally narrow: vendor, product name, board name,
ACPI ID/UID, and namespace path are matched where applicable.

## Configuration

Apply `p916f-config.fragment` after copying the running Ubuntu configuration,
then run `make olddefconfig`.  The release used for the test build was:

```text
7.0.14-p916f1
```

Build artifacts are generated with:

```sh
make -j"$(nproc)" bindeb-pkg KDEB_PKGVERSION=7.0.14-p916f1-1
```

Install only the image and matching headers.  Do not remove the Ubuntu generic
kernel until the custom kernel has survived boot, suspend/resume, repeated
touchpad use, and WLAN power-cycle testing.

## Runtime verification

After booting the custom kernel:

```sh
uname -r
sudo journalctl -b -k | grep -E 'P916F|lost arbitration|controller timed out|AE_ALREADY_EXISTS'
sudo dmesg | grep -F 'I2C bus managed by AMD PSP'
```

Expected release is `7.0.14-p916f1`.  The log should contain the two P916F
compatibility messages and no recurring I2C arbitration or WLAN namespace
errors.
