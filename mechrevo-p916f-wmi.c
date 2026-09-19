// SPDX-License-Identifier: GPL-2.0-only
/*
 * Experimental WMI hotkey driver for the MECHREVO XINGYAO P916F.
 *
 * This machine reuses Huawei WMI GUIDs with an incompatible ACPI ABI.
 * Keep this driver strictly DMI-scoped and avoid firmware writes until the
 * platform method contract has been decoded from its ACPI tables.
 */

#include <linux/dmi.h>
#include <linux/input.h>
#include <linux/input/sparse-keymap.h>
#include <linux/module.h>
#include <linux/wmi.h>

#define P916F_EVENT_GUID "ABBC0F5C-8EA1-11D1-A000-C90629100000"

struct p916f_wmi {
	struct input_dev *input;
};

static const struct dmi_system_id p916f_dmi_table[] = {
	{
		.ident = "MECHREVO XINGYAO Series-P916F-HPT-R",
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "MECHREVO"),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME, "XINGYAO Series"),
			DMI_EXACT_MATCH(DMI_BOARD_NAME,
					"XINGYAO Series-P916F-HPT-R"),
		},
	},
	{}
};
MODULE_DEVICE_TABLE(dmi, p916f_dmi_table);

/* Event values assigned to WMEN by the P916F DSDT EC query methods. */
static const struct key_entry p916f_keymap[] = {
	{ KE_KEY, 0x20, { KEY_KBDILLUMDOWN } }, /* backlight off */
	{ KE_KEY, 0x21, { KEY_KBDILLUMUP } },   /* backlight level 1 */
	{ KE_KEY, 0x22, { KEY_KBDILLUMUP } },   /* backlight level 2 */
	{ KE_KEY, 0x30, { KEY_TOUCHPAD_OFF } },
	{ KE_KEY, 0x31, { KEY_TOUCHPAD_ON } },
	{ KE_KEY, 0x41, { KEY_PROG1 } },        /* balanced profile */
	{ KE_KEY, 0x42, { KEY_PROG1 } },        /* performance profile */
	{ KE_END, 0 }
};

static void p916f_wmi_notify(struct wmi_device *wdev, union acpi_object *obj)
{
	struct p916f_wmi *priv = dev_get_drvdata(&wdev->dev);
	const struct key_entry *key;
	u32 code;

	if (!obj || obj->type != ACPI_TYPE_INTEGER) {
		dev_warn_ratelimited(&wdev->dev,
				     "event payload is not an ACPI integer\n");
		return;
	}

	code = obj->integer.value;
	key = sparse_keymap_entry_from_scancode(priv->input, code);
	if (!key) {
		dev_info_ratelimited(&wdev->dev,
				     "unknown hotkey event 0x%08x\n", code);
		return;
	}

	sparse_keymap_report_entry(priv->input, key, 1, true);
}

static int p916f_wmi_probe(struct wmi_device *wdev, const void *context)
{
	struct device *dev = &wdev->dev;
	struct p916f_wmi *priv;
	int ret;

	if (!dmi_check_system(p916f_dmi_table))
		return -ENODEV;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->input = devm_input_allocate_device(dev);
	if (!priv->input)
		return -ENOMEM;

	priv->input->name = "MECHREVO P916F hotkeys";
	priv->input->phys = "wmi/input0";
	priv->input->id.bustype = BUS_HOST;

	ret = sparse_keymap_setup(priv->input, p916f_keymap, NULL);
	if (ret)
		return ret;

	ret = input_register_device(priv->input);
	if (ret)
		return ret;

	dev_set_drvdata(dev, priv);
	dev_info(dev, "registered safe event-only support\n");
	return 0;
}

static const struct wmi_device_id p916f_wmi_id_table[] = {
	{ P916F_EVENT_GUID, NULL },
	{}
};
MODULE_DEVICE_TABLE(wmi, p916f_wmi_id_table);

static struct wmi_driver p916f_wmi_driver = {
	.driver = {
		.name = "mechrevo-p916f-wmi",
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
	},
	.id_table = p916f_wmi_id_table,
	.probe = p916f_wmi_probe,
	.notify = p916f_wmi_notify,
};
module_wmi_driver(p916f_wmi_driver);

MODULE_AUTHOR("chenghao <longwudf@gmail.com>");
MODULE_DESCRIPTION("MECHREVO XINGYAO P916F WMI hotkey driver");
MODULE_LICENSE("GPL");
