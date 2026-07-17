// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com
 *
 * Samsung TN debugging code
 *
 */

#include <linux/of.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/uio.h>
#include <linux/vmalloc.h>
#include <linux/sec_debug.h>
#include "sec_debug_internal.h"


static unsigned long dhist_base;
static unsigned long dhist_size;

static ssize_t secdbg_hist_read(struct file *file, char __user *buf,
				  size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	ssize_t count, ret = 0;
	char *base = NULL;

	if (!dhist_size) {
		pr_crit("%s: size 0? %lx\n", __func__, dhist_size);

		ret = -ENXIO;

		goto fail;
	}

	if (!dhist_base) {
		pr_crit("%s: no base? %lx\n", __func__, dhist_base);

		ret = -ENXIO;

		goto fail;
	}

	if (pos >= dhist_size) {
		pr_crit("%s: pos %llx , dhist: %lx\n", __func__, pos, dhist_size);

		ret = 0;

		goto fail;
	}

	count = min(len, (size_t)(dhist_size - pos));

	base = (char *)phys_to_virt((phys_addr_t)dhist_base);
	if (!base) {
		pr_crit("%s: fail to get va (%lx)\n", __func__, dhist_base);

		ret = -EFAULT;

		goto fail;
	}

	if (copy_to_user(buf, base + pos, count)) {
		pr_crit("%s: fail to copy to use\n", __func__);

		ret = -EFAULT;
	} else {
		pr_debug("%s: base: %p\n", __func__, base);

		*offset += count;
		ret = count;
	}

fail:
	return ret;
}

static const struct file_operations dhist_file_ops = {
	.owner = THIS_MODULE,
	.read = secdbg_hist_read,
};

static unsigned long summ_base;
static unsigned long summ_size;

static ssize_t secdbg_summ_read(struct file *file, char __user *buf,
				  size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	ssize_t count, ret = 0;
	char *base = NULL;

	if (!summ_size) {
		pr_crit("%s: size 0? %lx\n", __func__, summ_size);

		ret = -ENXIO;

		goto fail;
	}

	if (!summ_base) {
		pr_crit("%s: no base? %lx\n", __func__, summ_base);

		ret = -ENXIO;

		goto fail;
	}

	if (pos >= summ_size) {
		pr_crit("%s: pos %llx , summ_size: %lx\n", __func__, pos, summ_size);

		ret = 0;

		goto fail;
	}

	count = min(len, (size_t)(summ_size - pos));

	base = (char *)phys_to_virt((phys_addr_t)summ_base);
	if (!base) {
		pr_crit("%s: fail to get va (%lx)\n", __func__, summ_base);

		ret = -EFAULT;

		goto fail;
	}

	if (copy_to_user(buf, base + pos, count)) {
		pr_crit("%s: fail to copy to use\n", __func__);

		ret = -EFAULT;
	} else {
		*offset += count;
		ret = count;
	}

fail:
	return ret;
}


static const struct file_operations summ_file_ops = {
	.owner = THIS_MODULE,
	.read = secdbg_summ_read,
};

static int __init secdbg_node_init(void)
{
	struct proc_dir_entry *dhst_entry;
	struct proc_dir_entry *summ_entry;
	char *base;

	dhist_base = secdbg_base_get_buf_base(SDN_MAP_DEBUG_PARAM);
	base = (char *)phys_to_virt((phys_addr_t)dhist_base);
	dhist_size = secdbg_base_get_buf_size(SDN_MAP_DEBUG_PARAM);

	dhst_entry = proc_create("debug_history", S_IFREG | 0444, NULL, &dhist_file_ops);
	if (!dhst_entry) {
		pr_err("%s: failed to create proc entry debug part debug history log\n", __func__);
		return 0;
	}

	summ_base = secdbg_base_get_buf_base(SDN_MAP_DUMP_SUMMARY);
	base = (char *)phys_to_virt((phys_addr_t)summ_base);
	summ_size = secdbg_base_get_buf_size(SDN_MAP_DUMP_SUMMARY);

	summ_entry = proc_create("reset_summary", S_IFREG | 0444, NULL, &summ_file_ops);
	if (!summ_entry) {
		pr_err("%s: failed to create proc entry debug part dump summary\n", __func__);
		return 0;
	}

	pr_info("%s: success to create proc entry\n", __func__);

	proc_set_size(dhst_entry, (size_t)dhst_size);
	proc_set_size(summ_entry, (size_t)summ_size);

	return 0;
}
module_init(secdbg_node_init);

static void __exit secdbg_node_exit(void)
{
}
module_exit(secdbg_node_exit);

MODULE_DESCRIPTION("Samsung Debug Node driver");
MODULE_LICENSE("GPL v2");
