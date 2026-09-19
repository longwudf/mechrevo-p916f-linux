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
#include <linux/unaligned.h>
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

/* Codes already emitted through the shared event GUID on this firmware. */
static const struct key_entry p916f_keymap[] = {
	{ KE_KEY, 0x281, { KEY_BRIGHTNESSDOWN } },
	{ KE_KEY, 0x282, { KEY_BRIGHTNESSUP } },
	{ KE_KEY, 0x284, { KEY_MUTE } },
	{ KE_KEY, 0x285, { KEY_VOLUMEDOWN } },
	{ KE_KEY, 0x286, { KEY_VOLUMEUP } },
	{ KE_KEY, 0x287, { KEY_MICMUTE } },
	{ KE_KEY, 0x289, { KEY_WLAN } },
	{ KE_KEY, 0x28a, { KEY_PROG1 } },
	{ KE_KEY, 0x28e, { KEY_PRINT } },
	{ KE_KEY, 0x293, { KEY_KBDILLUMTOGGLE } },
	{ KE_KEY, 0x294, { KEY_KBDILLUMDOWN } },
	{ KE_KEY, 0x295, { KEY_KBDILLUMUP } },
	{ KE_END, 0 }
};

static void p916f_wmi_notify(struct wmi_device *wdev,
			     const struct wmi_buffer *data)
{
	struct p916f_wmi *priv = dev_get_drvdata(&wdev->dev);
	const struct key_entry *key;
	u32 code;

	if (!data || data->length < sizeof(u32)) {
		dev_warn_ratelimited(&wdev->dev,
				     "event payload is missing or too short\n");
		return;
	}

	code = get_unaligned_le32(data->data);
	key = sparse_keymap_entry_from_scancode(priv->input, code);
	if (!key) {
		dev_info_ratelimited(&wdev->dev,
				     "unknown hotkey event 0x%08x (length %zu)\n",
				     code, data->length);
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
	.notify_new = p916f_wmi_notify,
};
module_wmi_driver(p916f_wmi_driver);

MODULE_AUTHOR("longwudf <longwudf@gmail.com>");
MODULE_DESCRIPTION("MECHREVO XINGYAO P916F WMI hotkey driver");
MODULE_LICENSE("GPL");
