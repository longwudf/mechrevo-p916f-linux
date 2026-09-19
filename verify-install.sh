#!/bin/sh
set -eu

status=0

if lsmod | grep -q '^mechrevo_p916f_wmi '; then
	echo "PASS: mechrevo_p916f_wmi is loaded"
else
	echo "FAIL: mechrevo_p916f_wmi is not loaded"
	status=1
fi

if lsmod | grep -q '^huawei_wmi '; then
	echo "FAIL: conflicting huawei_wmi is loaded"
	status=1
else
	echo "PASS: conflicting huawei_wmi is not loaded"
fi

if grep -lqx 'MECHREVO P916F hotkeys' /sys/class/input/input*/name \
	>/dev/null 2>&1; then
	echo "PASS: P916F hotkey input device exists"
else
	echo "FAIL: P916F hotkey input device was not found"
	status=1
fi

exit "$status"
