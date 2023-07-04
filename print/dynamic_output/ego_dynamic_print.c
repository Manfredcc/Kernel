#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

static bool debug_option = true;    /* hard-code control */

#define ego_debug(chip, fmt, ...)    \
    do {                            \
        if (chip->debug_on && debug_option)        \
            pr_debug("%s: %s " fmt, chip->name, \
                __func__, ##__VA_ARGS__);       \
        else                                    \
            ;   \
    } while(0)

typedef struct _egoist {
    char *name;
    struct hrtimer hrtimer;
    unsigned long relative_time;
    bool debug_on;
}egoist, *pegoist;
pegoist chip;

enum hrtimer_restart hrtimer_callback(struct hrtimer *timer)
{
    pegoist dev = container_of(timer, egoist, hrtimer);
    ego_debug(dev, "Called\n");

    hrtimer_forward_now(timer, chip->relative_time);
    return HRTIMER_RESTART;
}

void ego_release(pegoist chip)
{
    if (chip != NULL) {
        /* Nothins to do in this module */
    } else {
        pr_err("Failed to alloc mem for egoist\n");
    }
}

static int __init ego_dynamic_print_init(void)
{
    int ret = 0;
    ktime_t ktime;

    do {
        chip = kzalloc(sizeof(*chip), GFP_KERNEL);
        if (!chip) {
            ret = ENOMEM;
            break;
        }

        chip->name = "egoist";
        chip->debug_on = true;
        chip->relative_time = 500000000; /* 500ms */
        ktime = ktime_set(0, chip->relative_time);
        hrtimer_init(&chip->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        chip->hrtimer.function = &hrtimer_callback;

        hrtimer_start(&chip->hrtimer, ktime, HRTIMER_MODE_REL);
    } while (0);

    if (ret) {
        ego_release(chip);
        return ret;
    }
    
    ego_debug(chip, "All things goes well, awesome\n");
    return ret;
}

static void __exit ego_dynamic_print_exit(void)
{
    int ret;

    ego_release(chip);
    ret = hrtimer_cancel(&chip->hrtimer);
    if (ret) {
        pr_info("The timer was still in use...\n");
    }

    pr_info("All things gone\n");
}

module_init(ego_dynamic_print_init);
module_exit(ego_dynamic_print_exit);

MODULE_AUTHOR("Manfred <1259106665@qq.com>");
MODULE_LICENSE("GPL");
