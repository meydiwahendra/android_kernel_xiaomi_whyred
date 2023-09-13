/*
 * Author: Meydi Wahendra <meydiwahendra@gmail.com> Copyright (c) 2023
 * 
 * License: GPL
 * A simple modules to optimize the custom kernel of Whyred (Redmi Note 5 Pro)
 * 
 * Adapted fastchg code to this modules. 
 * Author: Chad Froebel <chadfroebel@gmail.com>
 *
 * Adapted dsboost code to this module.
 * Author: Tyler Nijmeh <tylernij@gmail.com> Copyright (c) 2019
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
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/input.h>
#include <linux/sysfs.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>

#include <linux/deopt.h>

static struct kobject *deopt_kobj;
static struct workqueue_struct *deoptboost_wq;

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

static struct work_struct input_boost_work;
static struct delayed_work input_boost_rem;

static unsigned short input_boost_duration = CONFIG_INPUT_BOOST_DURATION;
static unsigned short input_stune_boost = CONFIG_INPUT_STUNE_BOOST;
static unsigned short sched_stune_boost = CONFIG_SCHED_STUNE_BOOST;

module_param(input_boost_duration, ushort, 0644);
module_param(input_stune_boost, ushort, 0644);
module_param(sched_stune_boost, ushort, 0644);

static ssize_t input_boost_duration_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%u\n", input_boost_duration);
}

static ssize_t input_boost_duration_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    unsigned short value;
    int ret = sscanf(buf, "%hu", &value);
    if (ret == 1) {
        input_boost_duration = value;
        return count;
    }
    return -EINVAL;
}

static ssize_t input_stune_boost_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%u\n", input_stune_boost);
}

static ssize_t input_stune_boost_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    unsigned short value;
    int ret = sscanf(buf, "%hu", &value);
    if (ret == 1) {
        input_stune_boost = value;
        return count;
    }
    return -EINVAL;
}

static ssize_t sched_stune_boost_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%u\n", sched_stune_boost);
}

static ssize_t sched_stune_boost_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    unsigned short value;
    int ret = sscanf(buf, "%hu", &value);
    if (ret == 1) {
        sched_stune_boost = value;
        return count;
    }
    return -EINVAL;
}

static struct kobj_attribute input_boost_duration_attr = {
    .attr = { .name = "input_boost_duration", .mode = 0644 },
    .show = input_boost_duration_show,
    .store = input_boost_duration_store,
};

static struct kobj_attribute input_stune_boost_attr = {
    .attr = { .name = "input_stune_boost", .mode = 0644 },
    .show = input_stune_boost_show,
    .store = input_stune_boost_store,
};

static struct kobj_attribute sched_stune_boost_attr = {
    .attr = { .name = "sched_stune_boost", .mode = 0644 },
    .show = sched_stune_boost_show,
    .store = sched_stune_boost_store,
};

static bool input_stune_boost_active;
static bool sched_stune_boost_active;

static u64 last_input_time;

/* How long after an input before another input boost can be triggered */
#define MIN_INPUT_INTERVAL (input_boost_duration * USEC_PER_MSEC)

static void do_input_boost_rem(struct work_struct *work)
{
    if (input_stune_boost_active)
        input_stune_boost_active = false;
}

static void do_input_boost(struct work_struct *work)
{
    if (!cancel_delayed_work_sync(&input_boost_rem))
    {
        if (!input_stune_boost_active)
            input_stune_boost_active = true;
    }

    queue_delayed_work(deoptboost_wq, &input_boost_rem, msecs_to_jiffies(input_boost_duration));
}

void do_sched_boost_rem(void)
{
    if (sched_stune_boost_active)
        sched_stune_boost_active = false;
}

void do_sched_boost(void)
{
    if (!sched_stune_boost)
        return;

    if (!sched_stune_boost_active)
        sched_stune_boost_active = true;
}

static void deserteagle_opt_input_event(struct input_handle *handle,
    unsigned int type, unsigned int code, int value)
{
    u64 now;

    now = ktime_to_us(ktime_get());
    if (now - last_input_time < MIN_INPUT_INTERVAL)
        return;

    if (work_pending(&input_boost_work))
        return;

    if (input_stune_boost)
        queue_work(deoptboost_wq, &input_boost_work);
    last_input_time = ktime_to_us(ktime_get());;
}

static int deserteagle_opt_input_connect(struct input_handler *handler,
		struct input_dev *dev, const struct input_device_id *id)
{
	struct input_handle *handle;
	int error;

	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "cpufreq";

	error = input_register_handle(handle);
	if (error)
		goto err2;

	error = input_open_device(handle);
	if (error)
		goto err1;

	return 0;
err1:
	input_unregister_handle(handle);
err2:
	kfree(handle);
	return error;
}

static void deserteagle_opt_input_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

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
	&input_boost_duration_attr.attr,
	&input_stune_boost_attr.attr,
	&sched_stune_boost_attr.attr,
	NULL,
};

static struct attribute_group deserteagle_opt_attr_group = {
	.attrs = deserteagle_opt_attrs,
};

static const struct input_device_id deserteagle_opt_ids[] = {
	/* multi-touch touchscreen */
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT |
			INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT_MASK(EV_ABS) },
		.absbit = { [BIT_WORD(ABS_MT_POSITION_X)] =
			BIT_MASK(ABS_MT_POSITION_X) |
			BIT_MASK(ABS_MT_POSITION_Y) },
	},
	/* touchpad */
	{
		.flags = INPUT_DEVICE_ID_MATCH_KEYBIT |
			INPUT_DEVICE_ID_MATCH_ABSBIT,
		.keybit = { [BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH) },
		.absbit = { [BIT_WORD(ABS_X)] =
			BIT_MASK(ABS_X) | BIT_MASK(ABS_Y) },
	},
	/* Keypad */
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) },
	},
    { },
};

static struct input_handler deserteagle_input_handler = {
    .event          = deserteagle_opt_input_event,
    .connect        = deserteagle_opt_input_connect,
    .disconnect     = deserteagle_opt_input_disconnect,
    .name           = "deserteagle_opt",
    .id_table       = deserteagle_opt_ids,
};

static int __init deserteagle_opt_init(void)
{
    int ret;

    // Create kobject path to /sys/kernel/deopt
    deopt_kobj = kobject_create_and_add("deopt", kernel_kobj);
    if (!deopt_kobj)
    {
        return -ENOMEM;
    }

    ret = sysfs_create_group(deopt_kobj, &deserteagle_opt_attr_group);
    if (ret)
    {
        kobject_put(deopt_kobj);
        return ret;
    }

    if (deserteagle_opt == 0)
    {
        pr_info("deserteagle_opt is disabled via /sys/kernel/deopt/deserteagle_opt. To enable it, please modify the value in /sys/kernel/deopt/deserteagle_opt.\n");
    }
    else
    {
        pr_info("deserteagle_opt is %s, check in /sys/kernel/deopt/\n", deserteagle_opt ? "enabled" : "disabled");
    }

    // Initialize deoptboost_wq
    deoptboost_wq = create_singlethread_workqueue("deoptboost_wq");
    if (!deoptboost_wq) {
        ret = -ENOMEM;
        goto err_wq;
    }

    INIT_WORK(&input_boost_work, do_input_boost);
    INIT_DELAYED_WORK(&input_boost_rem, do_input_boost_rem);

    ret = input_register_handler(&deserteagle_input_handler);
    if (ret)
        goto err_input;

    return 0;

err_input:
    input_unregister_handler(&deserteagle_input_handler);
err_wq:
    destroy_workqueue(deoptboost_wq);

    return ret;
}

static void __exit deserteagle_opt_exit(void)
{
    input_unregister_handler(&deserteagle_input_handler);
    destroy_workqueue(deoptboost_wq);
    if (deopt_kobj)
        kobject_put(deopt_kobj);
}

module_init(deserteagle_opt_init);
module_exit(deserteagle_opt_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Meydi Wahendra <meydiwahendra@gmail.com>");
MODULE_DESCRIPTION("Optimization module for custom kernel");
