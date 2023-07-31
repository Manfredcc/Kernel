#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fs.h>

static bool debug_option = true;    /* hard-code control */

#define ego_err(chip, fmt, ...)     \
    pr_err("%s: %s " fmt, chip->name,   \
        __func__, ##__VA_ARGS__)

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
    struct kobject kobj;
    unsigned long obj_val;
    bool debug_on;
}egoist, *pegoist;
pegoist chip;

void ego_release(pegoist chip)
{
    if (chip != NULL) {
        if (&chip->kobj) {
            kobject_put(&chip->kobj);
        }
        kfree(chip);
    } else {
        pr_err("Failed to alloc mem for egoist\n");
    }
}

static int demo_val;
ssize_t	demo_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    pegoist chip = container_of(kobj, egoist, kobj);
    
    return scnprintf(buf, PAGE_SIZE, "cc:%ld\n", chip->obj_val);
}

ssize_t	demo_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t cout)
{
    int ret;
    pegoist chip = container_of(kobj, egoist, kobj);
    ego_info(chip, "called\n");
    ret = kstrtoul(buf, 0, &chip->obj_val);
    if (ret)
        return ret;
    
    demo_val = chip->obj_val;

    return cout;
}

static struct sysfs_ops demo_ops = {
    .show = demo_show,
    .store = demo_store,
};

static struct attribute demo_1 = {
    .name = "demo_1",
    .mode = 0664,
};

static struct attribute demo_2 = {
    .name = "demo_2",
    .mode = 0664,
};

struct attribute *attr[] = {
    &demo_1,
    &demo_2,
    NULL
};

static const struct attribute_group attr_group = {
    .name = "attr_group",
    .attrs = attr,
};

static const struct attribute_group *overall_groups = &attr_group;

static struct kobj_type k_type = {
    .sysfs_ops = &demo_ops,
    .default_groups = &overall_groups,
};

static int __init ego_kobject_init(void)
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

        chip->kobj.ktype = &k_type;
        chip->kobj.kset = kset_create_and_add("ego_kset", NULL, NULL);
        ret = kobject_init_and_add(&chip->kobj, chip->kobj.ktype, NULL, "%s", chip->name);
        if (ret) {
            ego_err(chip, "Could not register\n");
            break;
        }

        kobject_uevent(&chip->kobj, KOBJ_CHANGE);

    } while (0);

    if (ret) {
        ego_release(chip);
        return ret;
    }
    
    ego_info(chip, "All things goes well, awesome\n");
    return ret;
}

static void __exit ego_kobject_exit(void)
{
    ego_release(chip);
    pr_info("All things gone\n");
}

module_init(ego_kobject_init);
module_exit(ego_kobject_exit);

MODULE_AUTHOR("Manfred <1259106665@qq.com>");
MODULE_LICENSE("GPL");
