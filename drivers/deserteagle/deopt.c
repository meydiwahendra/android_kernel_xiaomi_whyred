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

static void set_sysfs_value(const char *path, const char *value, mode_t mode) {
    struct file *file;
    mm_segment_t oldfs;
    loff_t pos = 0;
    char sysfs_path[256];

    snprintf(sysfs_path, sizeof(sysfs_path), "%s", path);

    // Open the file with write permissions and create it if it doesn't exist
    file = filp_open(sysfs_path, O_WRONLY | O_CREAT, mode);
    if (IS_ERR(file)) {
        pr_err("Failed to open %s\n", sysfs_path);
        return;
    }

    oldfs = get_fs();
    set_fs(get_ds());
    vfs_write(file, value, strlen(value), &pos);
    set_fs(oldfs);

    filp_close(file, NULL);
}

static int __init deopt_init(void) {
    // Set sysfs values with permissions (e.g., 0644 for owner read/write and others read)
    set_sysfs_value("1", "/sys/kernel/fast_charge/force_fast_charge", 0644);
    set_sysfs_value("1", "/sys/class/power_supply/battery/system_temp_level", 0644);
    set_sysfs_value("1", "/sys/kernel/fast_charge/failsafe", 0644);
    set_sysfs_value("1", "/sys/class/power_supply/battery/allow_hvdcp3", 0644);
    set_sysfs_value("1", "/sys/class/power_supply/usb/pd_allowed", 0644);
    set_sysfs_value("1", "/sys/class/power_supply/battery/subsystem/usb/pd_allowed", 0644);
    set_sysfs_value("0", "/sys/class/power_supply/battery/input_current_limited", 0644);
    set_sysfs_value("1", "/sys/class/power_supply/battery/input_current_settled", 0644);
    set_sysfs_value("0", "/sys/class/qcom-battery/restricted_charging", 0644);
    set_sysfs_value("150", "/sys/class/power_supply/bms/temp_cool", 0644);
    set_sysfs_value("480", "/sys/class/power_supply/bms/temp_hot", 0644);
    set_sysfs_value("480", "/sys/class/power_supply/bms/temp_warm", 0644);

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
MODULE_DESCRIPTION("Kernel module to set sysfs values");
