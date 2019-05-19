#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

static int my_sys;
static int my_obj_show_counter=0;

static ssize_t my_show(struct kobject *kobj, struct kobj_attribute *attr,
                       char *buf)
{
    my_obj_show_counter++;
    int bytes = sprintf(buf, "%d", my_obj_show_counter);
    return bytes;
}

static ssize_t my_store(struct kobject *kobj, struct kobj_attribute *attr,
                        const char *buf, size_t count)
{
    return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute my_attribute =
        __ATTR(my_sys, 0755, my_show, my_store);

static struct kobject *example_kobj;

static int __init example_init(void)
{
    int retval;
    example_kobj = kobject_create_and_add("my_kobject", kernel_kobj);
    if (!example_kobj)
        return -ENOMEM;

    /* Create the files associated with this kobject */
    retval = sysfs_create_file(example_kobj, &my_attribute.attr);
    if (retval)
        kobject_put(example_kobj);

    return retval;
}

static void __exit example_exit(void)
{
    // kobject_put(example_kobj);
    kobject_del(example_kobj);

}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Chizhikov <nikita-chizh@yandex.ru>");
