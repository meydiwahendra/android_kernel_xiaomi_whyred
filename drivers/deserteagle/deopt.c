/*
 * Author: Meydi Wahendra <meydiwahendra@gmail.com>
 * License: GPL
 * A simple modules to optimize the custom kernel of Whyred (Redmi Note 5 Pro)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/power_supply.h>
#include "../../drivers/power/supply/qcom/smb-lib.h"

#define smblib_set_prop_input_current_settled(power_supply, val) \
    power_supply_set_property(power_supply, POWER_SUPPLY_PROP_INPUT_CURRENT_SETTLED, val)

#define smblib_set_prop_input_current_limited(power_supply, val) \
    power_supply_set_property(power_supply, POWER_SUPPLY_PROP_INPUT_CURRENT_LIMITED, val)

#define smblib_set_prop_restricted_charging(power_supply, val) \
    power_supply_set_property(power_supply, POWER_SUPPLY_PROP_RESTRICTED_CHARGING, val)

#define smblib_set_prop_cool_temp(power_supply, val) \
    power_supply_set_property(power_supply, POWER_SUPPLY_PROP_COOL_TEMP, val)

#define smblib_set_prop_warm_temp(power_supply, val) \
    power_supply_set_property(power_supply, POWER_SUPPLY_PROP_WARM_TEMP, val)

#define smblib_set_prop_hot_temp(power_supply, val) \
    power_supply_set_property(power_supply, POWER_SUPPLY_PROP_HOT_TEMP, val)

#define smblib_set_prop_pd_allowed(power_supply, val) \
    power_supply_set_property(power_supply, POWER_SUPPLY_PROP_PD_ALLOWED, val)

#define smblib_set_prop_allow_hvdcp3(power_supply, val) \
    power_supply_set_property(power_supply, POWER_SUPPLY_PROP_ALLOW_HVDCP3, val)

#define smblib_set_prop_system_temp_level(power_supply, val) \
    power_supply_set_property(power_supply, POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL, val)

static int __init deopt_init(void) {
    // Hardcoded values for power supply properties
    struct power_supply *psy = power_supply_get_by_name("battery");
    if (psy) {
        const union power_supply_propval input_current_settled_val = {
            .intval = 1
        };
        const union power_supply_propval input_current_limited_val = {
            .intval = 0
        };
        const union power_supply_propval restricted_charging_val = {
            .intval = 0
        };
        const union power_supply_propval cool_temp_val = {
            .intval = 150
        };
        const union power_supply_propval warm_temp_val = {
            .intval = 480
        };
        const union power_supply_propval hot_temp_val = {
            .intval = 480
        };
        const union power_supply_propval pd_allowed_val = {
            .intval = 1
        };
        const union power_supply_propval allow_hvdcp3_val = {
            .intval = 1
        };
        const union power_supply_propval system_temp_level_val = {
            .intval = 1
        };

        smblib_set_prop_input_current_settled(psy, &input_current_settled_val);
        smblib_set_prop_input_current_limited(psy, &input_current_limited_val);
        smblib_set_prop_restricted_charging(psy, &restricted_charging_val);
        smblib_set_prop_cool_temp(psy, &cool_temp_val);
        smblib_set_prop_warm_temp(psy, &warm_temp_val);
        smblib_set_prop_hot_temp(psy, &hot_temp_val);
        smblib_set_prop_pd_allowed(psy, &pd_allowed_val);
        smblib_set_prop_allow_hvdcp3(psy, &allow_hvdcp3_val);
        smblib_set_prop_system_temp_level(psy, &system_temp_level_val);
        power_supply_put(psy);
    }

    pr_info("deopt module loaded\n");

    return 0;
}

static void __exit deopt_exit(void) {
    pr_info("deopt module unloaded\n");
}

module_init(deopt_init);
module_exit(deopt_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Meydi Wahendra <meydiwahendra@gmail.com>");
MODULE_DESCRIPTION("Kernel module to set power supply properties");
