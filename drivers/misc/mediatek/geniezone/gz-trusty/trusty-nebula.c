// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*
 * GenieZone (hypervisor-based seucrity platform) enables hardware protected
 * and isolated security execution environment, includes
 * 1. GZ hypervisor
 * 2. Hypervisor-TEE OS (built-in Trusty OS)
 * 3. Drivers (ex: debug, communication and interrupt) for GZ and
 *    hypervisor-TEE OS
 * 4. GZ and hypervisor-TEE and GZ framework (supporting multiple TEE
 *    ecosystem, ex: M-TEE, Trusty, GlobalPlatform, ...)
 */
/* This is driver for Nebula hypervisor-TEE OS*/

#include <linux/random.h>
#include <gz-trusty/trusty.h>
#include <gz-trusty/smcall.h>


#ifdef CONFIG_MT_TRUSTY_DEBUGFS
ssize_t vmm_fast_add_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	s32 a, b, c, ret;

	get_random_bytes(&a, sizeof(s32));
	a &= 0xFF;
	get_random_bytes(&b, sizeof(s32));
	b &= 0xFF;
	get_random_bytes(&c, sizeof(s32));
	c &= 0xFF;
	ret = trusty_fast_call32(dev, SMC_FC_VM_TEST_ADD, a, b, c);
	return scnprintf(buf, PAGE_SIZE, "%d + %d + %d = %d, %s\n", a, b, c,
			 ret, (a + b + c) == ret ? "PASS" : "FAIL");
}

/*DEVICE_ATTR(vmm_fast_add, 0644, vmm_fast_add_show, NULL);*/
DEVICE_ATTR_RW(vmm_fast_add);


ssize_t vmm_fast_multiply_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	s32 a, b, c, ret;

	get_random_bytes(&a, sizeof(s32));
	a &= 0xFF;
	get_random_bytes(&b, sizeof(s32));
	b &= 0xFF;
	get_random_bytes(&c, sizeof(s32));
	c &= 0xFF;
	ret = trusty_fast_call32(dev, SMC_FC_VM_TEST_MULTIPLY, a, b, c);
	return scnprintf(buf, PAGE_SIZE, "%d * %d * %d = %d, %s\n", a, b, c,
			 ret, (a * b * c) == ret ? "PASS" : "FAIL");
}

/*DEVICE_ATTR(vmm_fast_multiply, 0644, vmm_fast_multiply_show, NULL);*/
DEVICE_ATTR_RW(vmm_fast_multiply);

ssize_t vmm_std_add_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	s32 a, b, c, ret;

	get_random_bytes(&a, sizeof(s32));
	a &= 0xFF;
	get_random_bytes(&b, sizeof(s32));
	b &= 0xFF;
	get_random_bytes(&c, sizeof(s32));
	c &= 0xFF;
	ret = trusty_std_call32(dev, SMC_SC_VM_TEST_ADD, a, b, c);
	return scnprintf(buf, PAGE_SIZE, "%d + %d + %d = %d, %s\n", a, b, c,
			 ret, (a + b + c) == ret ? "PASS" : "FAIL");
}

/*DEVICE_ATTR(vmm_std_add, 0644, vmm_std_add_show, NULL);*/
DEVICE_ATTR_RW(vmm_std_add);

ssize_t vmm_std_multiply_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	s32 a, b, c, ret;

	get_random_bytes(&a, sizeof(s32));
	a &= 0xFF;
	get_random_bytes(&b, sizeof(s32));
	b &= 0xFF;
	get_random_bytes(&c, sizeof(s32));
	c &= 0xFF;
	ret = trusty_std_call32(dev, SMC_SC_VM_TEST_MULTIPLY, a, b, c);
	return scnprintf(buf, PAGE_SIZE, "%d * %d * %d = %d, %s\n", a, b, c,
			 ret, (a * b * c) == ret ? "PASS" : "FAIL");
}

/*DEVICE_ATTR(vmm_std_multiply, 0644, vmm_std_multiply_show, NULL);*/
DEVICE_ATTR_RW(vmm_std_multiply);

void trusty_create_debugfs_vmm(struct trusty_state *s, struct device *pdev)
{
	int ret;

	ret = device_create_file(pdev, &dev_attr_vmm_fast_add);
	if (ret)
		goto err_create_vmm_fast_add;

	ret = device_create_file(pdev, &dev_attr_vmm_fast_multiply);
	if (ret)
		goto err_create_vmm_fast_multiply;

	ret = device_create_file(pdev, &dev_attr_vmm_std_add);
	if (ret)
		goto err_create_vmm_std_add;

	ret = device_create_file(pdev, &dev_attr_vmm_std_multiply);
	if (ret)
		goto err_create_vmm_std_multiply;

	return;

err_create_vmm_std_multiply:
	device_remove_file(pdev, &dev_attr_vmm_std_multiply);
err_create_vmm_std_add:
	device_remove_file(pdev, &dev_attr_vmm_std_add);
err_create_vmm_fast_multiply:
	device_remove_file(pdev, &dev_attr_vmm_fast_multiply);
err_create_vmm_fast_add:
	device_remove_file(pdev, &dev_attr_vmm_fast_add);
}
#endif				/* CONFIG_MT_TRUSTY_DEBUGFS */
