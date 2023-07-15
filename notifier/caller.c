#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/kthread.h>

extern struct raw_notifier_head ego_notifier;

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
    struct task_struct *task_caller;
    bool debug_on;
}egoist, *pegoist;
pegoist chip;

void ego_release(pegoist chip)
{
    if (chip != NULL) {
        if (!IS_ERR_OR_NULL(chip->task_caller)) {
            kthread_stop(chip->task_caller);
        }
        kfree(chip);
    } else {
        pr_err("Failed to alloc mem for egoist\n");
    }
}

static int caller_thread(void *data)
{
    ego_info(chip, "Enter\n");
    raw_notifier_call_chain(&ego_notifier, 0, NULL);
    ego_info(chip, "Exit\n");

    while (!kthread_should_stop())
        ;
    
    return 0;
}

static int __init ego_caller_init(void)
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

        chip->task_caller = kthread_run(&caller_thread, 0, "egoist_caller");

    } while (0);

    if (ret) {
        ego_release(chip);
        return ret;
    }
    
    ego_info(chip, "All things goes well, awesome\n");
    return ret;
}

static void __exit ego_caller_exit(void)
{
    ego_release(chip);
    pr_info("All things gone\n");
}

module_init(ego_caller_init);
module_exit(ego_caller_exit);

MODULE_AUTHOR("Manfred <1259106665@qq.com>");
MODULE_LICENSE("GPL");
