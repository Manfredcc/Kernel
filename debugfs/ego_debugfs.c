#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/workqueue.h>

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
    struct dentry *ego_dir;
    u8 test_u8;
    struct delayed_work d_work;
    bool debug_on;
}egoist, *pegoist;
pegoist chip;

static void print_work_handle(struct work_struct *work)
{
    pegoist dev = container_of(work, egoist, d_work.work);
    ego_info(dev, "Enter, test_u8=%d\n", dev->test_u8);
    schedule_delayed_work(&dev->d_work, 4 * HZ);
}

void ego_release(pegoist chip)
{
    if (chip != NULL) {
        cancel_delayed_work_sync(&chip->d_work);
        debugfs_remove_recursive(chip->ego_dir);
        kfree(chip);
    } else {
        pr_err("Failed to alloc mem for egoist\n");
    }
}

static int __init ego_print_init(void)
{
    int ret = 0;

    do {
        chip = kzalloc(sizeof(*chip), GFP_KERNEL);
        if (!chip) {
            ret = ENOMEM;
            break;
        }

        chip->name = "egoist";
        chip->debug_on = true;
        chip->ego_dir = debugfs_create_dir(chip->name, NULL);
        debugfs_create_u8("test_u8", 0660, chip->ego_dir, &chip->test_u8);
        INIT_DELAYED_WORK(&chip->d_work, &print_work_handle);

    } while (0);

    schedule_delayed_work(&chip->d_work, 0 * HZ);
    if (ret) {
        ego_release(chip);
        return ret;
    }
    
    ego_info(chip, "All things goes well, awesome\n");
    return ret;
}

static void __exit ego_print_exit(void)
{
    ego_release(chip);
    pr_info("All things gone\n");
}

module_init(ego_print_init);
module_exit(ego_print_exit);

MODULE_AUTHOR("Manfred <1259106665@qq.com>");
MODULE_LICENSE("GPL");
