#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
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
    struct semaphore sem;
    struct delayed_work sem_work;
    bool debug_on;
}egoist, *pegoist;
pegoist chip;

void ego_release(pegoist chip)
{
    if (chip != NULL) {
        up(&chip->sem);
        cancel_delayed_work(&chip->sem_work);
        kfree(chip);
    } else {
        pr_err("Failed to alloc mem for egoist\n");
    }
}

static void sem_work_handle(struct work_struct *work)
{
    pegoist dev = container_of(work, egoist, sem_work.work);

    ego_info(dev, "I'm coming, who should keep sync with END\n");
    up(&dev->sem);
}

static int __init ego_semaphore_init(void)
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
        sema_init(&chip->sem, 1);
        INIT_DELAYED_WORK(&chip->sem_work, sem_work_handle);

    } while (0);

    ego_info(chip, "I'm the headmos one\n");
    if (down_interruptible(&chip->sem)) {
        ret = EINTR;
    }
    schedule_delayed_work(&chip->sem_work, 5 * HZ);
    if (down_interruptible(&chip->sem)) {
        ret = EINTR;
    }
    ego_info(chip, "The END\n");
    up(&chip->sem);

    if (ret) {
        ego_release(chip);
        return ret;
    }
    
    ego_info(chip, "All things goes well, awesome\n");
    return ret;
}

static void __exit ego_semaphore_exit(void)
{
    ego_release(chip);
    pr_info("All things gone\n");
}

module_init(ego_semaphore_init);
module_exit(ego_semaphore_exit);

MODULE_AUTHOR("Manfred <1259106665@qq.com>");
MODULE_LICENSE("GPL");
