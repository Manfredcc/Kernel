#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>

static struct platform_device *pdev;

static int __init virtual_dev_init(void)
{
    int id = 0;
    pdev = platform_device_alloc("virtual_dev", id);
    platform_device_add(pdev);
    pr_info("virtual device added\n");
    return 0;
}

static void __exit virtual_dev_exit(void)
{
    pr_info("virtual_add device removed\n");
	platform_device_put(pdev);
}

module_init(virtual_dev_init);
module_exit(virtual_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manfred <1259106665@qq.com>");
