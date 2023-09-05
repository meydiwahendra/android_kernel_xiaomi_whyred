/*
 * Author: Meydi Wahendra <meydiwahendra@gmail.com>
 * License: GPL
 * A simple modules to optimize the custom kernel of Whyred (Redmi Note 5 Pro)
 * 
 * Adapted fastchg code to this modules. 
 * Author: Chad Froebel <chadfroebel@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/deopt.h>

static struct kobject *deopt_kobj;

int deserteagle_opt = 1;

static int __init get_fastcharge_opt(char *ffc)
{
	if (strcmp(ffc, "0") == 0) {
		deserteagle_opt = 0;
	} else if (strcmp(ffc, "1") == 0) {
		deserteagle_opt = 1;
	} else {
		deserteagle_opt = 0;
	}
	return 1;
}

__setup("ffc=", get_fastcharge_opt);

static ssize_t deserteagle_opt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	size_t count = 0;
	count += sprintf(buf, "%d\n", deserteagle_opt);
	return count;
}

static ssize_t deserteagle_opt_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d ", &deserteagle_opt);
	if (deserteagle_opt < 0 || deserteagle_opt > 1)
		deserteagle_opt = 0;

	return count;
}

static struct kobj_attribute deserteagle_opt_attribute = __ATTR(deserteagle_opt, 0664, deserteagle_opt_show, deserteagle_opt_store);

static struct attribute *deserteagle_opt_attrs[] = {
	&deserteagle_opt_attribute.attr,
	NULL,
};

static struct attribute_group deserteagle_opt_attr_group = {
	.attrs = deserteagle_opt_attrs,
};

/* Initialize deserteagle_opt sysfs folder */

int deserteagle_opt_init(void)
{
	int deserteagle_opt_retval;

	// Change the kobject path to /sys/kernel/deopt
	deopt_kobj = kobject_create_and_add("deopt", kernel_kobj);
	if (!deopt_kobj) {
		return -ENOMEM;
	}

	deserteagle_opt_retval = sysfs_create_group(deopt_kobj, &deserteagle_opt_attr_group);

	if (deserteagle_opt_retval)
		kobject_put(deopt_kobj);

	if (deserteagle_opt_retval)
		kobject_put(deopt_kobj);

	pr_info("deserteagle_opt is %s, check in /sys/kernel/deopt/\n", deserteagle_opt ? "enabled" : "disabled");

	return (deserteagle_opt_retval);
}

void deserteagle_opt_exit(void)
{
	kobject_put(deopt_kobj);
}

module_init(deserteagle_opt_init);
module_exit(deserteagle_opt_exit);
