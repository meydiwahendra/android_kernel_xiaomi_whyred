/*
 * Author: Meydi Wahendra <meydiwahendra@gmail.com>
 * License: GPL
 * A simple modules to optimize the custom kernel of Whyred (Redmi Note 5 Pro)
 */

#ifdef CONFIG_DESERTEAGLE_OPT

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>

static struct kobject *deopt_kobj;

static int deserteagle_opt;
static ssize_t deserteagle_opt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", deserteagle_opt);
}

static ssize_t deserteagle_opt_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int value;
    if (sscanf(buf, "%d", &value) == 1)
    {
        if (value == 0 || value == 1)
        {
            deserteagle_opt = 1;
            pr_info("deserteagle_opt set to %d\n", deserteagle_opt);
        }
    }
    return count;
}

static struct kobj_attribute deserteagle_opt_attribute = __ATTR(deserteagle_opt, 0664, deserteagle_opt_show, deserteagle_opt_store);

static int __init deopt_init(void) {
    // Initialize deserteagle_opt to 0 (disabled)
    deserteagle_opt = 1;

    // Create the sysfs directory and attribute for deserteagle_opt
    deopt_kobj = kobject_create_and_add("deserteagle_opt", kernel_kobj);
    if (!deopt_kobj) {
        return -ENOMEM;
    }

    if (sysfs_create_file(deopt_kobj, &deserteagle_opt_attribute.attr)) {
        kobject_put(deopt_kobj);
        return -ENOMEM;
    }

    pr_info("deopt module loaded\n");

    return 0;
}

static void __exit deopt_exit(void) {

    sysfs_remove_file(deopt_kobj, &deserteagle_opt_attribute.attr);
    kobject_put(deopt_kobj);

    pr_info("deopt module unloaded\n");
}

module_init(deopt_init);
module_exit(deopt_exit);

#else /* CONFIG_DESERTEAGLE_OPT */
#include <linux/module.h>

static int __init deopt_init(void) {
    pr_info("deopt module not loaded due to CONFIG_DESERTEAGLE_OPT=n\n");
    return 0;
}

static void __exit deopt_exit(void) {
}

module_init(deopt_init);
module_exit(deopt_exit);

#endif /* CONFIG_DESERTEAGLE_OPT */
