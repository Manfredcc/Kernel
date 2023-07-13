#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>

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
    struct tasklet_struct task;
    spinlock_t lock;
    unsigned long share_data;
    bool debug_on;
}egoist, *pegoist;
pegoist chip;

void ego_release(pegoist chip)
{
    if (chip != NULL) {
        kfree(chip);
        tasklet_kill(&chip->task);
    } else {
        pr_err("Failed to alloc mem for egoist\n");
    }
}

static void tasklet_handle(unsigned long data)
{
    unsigned long flags;

    ego_info(chip, "Enter, share_data=%lu\n", chip->share_data);

    spin_lock_irqsave(&chip->lock, flags);
    chip->share_data++;
    spin_unlock_irqrestore(&chip->lock, flags);

    ego_info(chip, "Over, share_data=%lu\n", chip->share_data);
}

static int __init ego_spinlock_init(void)
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
        spin_lock_init(&chip->lock);
        tasklet_init(&chip->task, tasklet_handle, chip->share_data);        

    } while (0);

    if (ret) {
        ego_release(chip);
        return ret;
    }

    tasklet_schedule(&chip->task);
    
    ego_info(chip, "All things goes well, awesome\n");
    return ret;
}

static void __exit ego_spinlock_exit(void)
{
    ego_release(chip);
    pr_info("All things gone\n");
}

module_init(ego_spinlock_init);
module_exit(ego_spinlock_exit);

MODULE_AUTHOR("Manfred <1259106665@qq.com>");
MODULE_LICENSE("GPL");
