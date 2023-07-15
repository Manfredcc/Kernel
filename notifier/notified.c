#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fs.h>

static bool debug_option = true;    /* hard-code control */

#define ego_err(chip, fmt, ...)     \
    pr_err("%s: %s " fmt, chip->name,   \
        __func__, ##__VA__ARGS__)

#define ego_info(chip, fmt, ...)    \
    do {                            \
        if (chip->debug_on && debug_option)        \
            pr_info("%s: %s " fmt, chip->name, \
                __func__, ##__VA_ARGS__);       \
        else                                    \
            ;   \
    } while(0)

typedef struct _egoist {
    char *name;
    struct notifier_block notifier_2;
    struct notifier_block notifier_1;
    struct notifier_block notifier_3;
    bool debug_on;
}egoist, *pegoist;
pegoist chip;

RAW_NOTIFIER_HEAD(ego_notifier);
EXPORT_SYMBOL_GPL(ego_notifier);

int notifier_1_callback(struct notifier_block *nb, unsigned long action, void *data)
{
    pegoist dev = container_of(nb, egoist, notifier_1);

    ego_info(dev, "Notified!\n");
    return 0;
}

int notifier_2_callback(struct notifier_block *nb, unsigned long action, void *data)
{
    pegoist dev = container_of(nb, egoist, notifier_2);

    ego_info(dev, "Notified!\n");
    return 0;

}

int notifier_3_callback(struct notifier_block *nb, unsigned long action, void *data)
{
    pegoist dev = container_of(nb, egoist, notifier_3);

    ego_info(dev, "Notified!\n");
    return 0;
}

void ego_release(pegoist chip)
{
    if (chip != NULL) {
        raw_notifier_chain_unregister(&ego_notifier, &chip->notifier_1);
        raw_notifier_chain_unregister(&ego_notifier, &chip->notifier_2);
        raw_notifier_chain_unregister(&ego_notifier, &chip->notifier_3);
        kfree(chip);
    } else {
        pr_err("Failed to alloc mem for egoist\n");
    }
}

void chip_init(pegoist chip)
{
    chip->name = "egoist";
    chip->debug_on = true;
    chip->notifier_1.notifier_call = &notifier_1_callback;
    chip->notifier_2.notifier_call = &notifier_2_callback;
    chip->notifier_3.notifier_call = &notifier_3_callback;

    chip->notifier_3.priority = 1;

    raw_notifier_chain_register(&ego_notifier, &chip->notifier_1);
    raw_notifier_chain_register(&ego_notifier, &chip->notifier_2);
    raw_notifier_chain_register(&ego_notifier, &chip->notifier_3);

}

static int __init ego_notified_init(void)
{
    int ret = 0;

    do {
        chip = kzalloc(sizeof(*chip), GFP_KERNEL);
        if (!chip) {
            ret = ENOMEM;
            break;
        }
    chip_init(chip);

    } while (0);

    if (ret) {
        ego_release(chip);
        return ret;
    }
    
    ego_info(chip, "All things goes well, awesome\n");
    return ret;
}

static void __exit ego_notified_exit(void)
{
    ego_release(chip);
    pr_info("All things gone\n");
}

module_init(ego_notified_init);
module_exit(ego_notified_exit);

MODULE_AUTHOR("Manfred <1259106665@qq.com>");
MODULE_LICENSE("GPL");
