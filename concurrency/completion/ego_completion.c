#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/delay.h>

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
    struct completion ack;
    struct delayed_work thread_wake;
    struct task_struct *thread_waiter_1;
    struct task_struct *thread_waiter_2;
    bool debug_on;
}egoist, *pegoist;
pegoist chip;

void ego_release(pegoist chip)
{
    if (chip != NULL) {
        cancel_delayed_work(&chip->thread_wake);
        if (!IS_ERR_OR_NULL(chip->thread_waiter_1)) {
            kthread_stop(chip->thread_waiter_1);
        }

        if (!IS_ERR_OR_NULL(chip->thread_waiter_2)) {
            kthread_stop(chip->thread_waiter_2);
        }
        kfree(chip);
    } else {
        pr_err("Failed to alloc mem for egoist\n");
    }
}

static void wake_handle(struct work_struct *work)
{
    pegoist dev = container_of(work, egoist, thread_wake.work);

    ego_info(dev, "Enter and ready to use complete\n");
    complete(&dev->ack);
    mdelay(2000);
    ego_info(dev, "The second time\n");
    complete(&dev->ack);
}

static int waiter_1_thread(void *arg)
{
    ego_info(chip, "Enter\n");
    wait_for_completion(&chip->ack);
    ego_info(chip, "Exit\n");

    while(!kthread_should_stop())
        ;

    return 0;
}

static int waiter_2_thread(void *arg)
{
    ego_info(chip, "Enter\n");
    wait_for_completion(&chip->ack);
    ego_info(chip, "Exit\n");

    while(!kthread_should_stop())
        ;

    return 0;
}


static int __init ego_completion_init(void)
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
        init_completion(&chip->ack); /* Initialize a completon */
        INIT_DELAYED_WORK(&chip->thread_wake, wake_handle);
        chip->thread_waiter_1 = kthread_run(waiter_1_thread, NULL, "waiter_1");
        mdelay(1000);
        chip->thread_waiter_2 = kthread_run(waiter_2_thread, NULL, "waiter_2");

    } while (0);

    schedule_delayed_work(&chip->thread_wake, 0 * HZ);

    if (ret) {
        ego_release(chip);
        return ret;
    }
    
    ego_info(chip, "All things goes well, awesome\n");
    return ret;
}

static void __exit ego_completion_exit(void)
{
    ego_release(chip);
    pr_info("All things gone\n");
}

module_init(ego_completion_init);
module_exit(ego_completion_exit);

MODULE_AUTHOR("Manfred <1259106665@qq.com>");
MODULE_LICENSE("GPL");
