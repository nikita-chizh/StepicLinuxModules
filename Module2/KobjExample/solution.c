#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

/*
 * This module shows how to create a simple subdirectory in sysfs called
 * /sys/kernel/kobject-example  In that directory, 3 files are created:
 * "foo", "baz", and "bar".  If an integer is written to these files, it can be
 * later read out of it.
 */

static int foo;
static int baz;
static int bar;
//
static int foo_show_counter=0;
static int foo_store_counter=0;
//
static int baz_show_counter=0;
static int baz_store_counter=0;
//
static int bar_show_counter=0;
static int bar_store_counter=0;


/*
 * foo_show дергается каждый раз при чтении (cat, nano) foo
 * При этом если делать это гуем дергается при открытии папки в гуи
 * и несколько раз при даблклике на файл
 * buf -> то что вернется в userspace, размер не должен превышать PAGE_SIZE
 */
// printk( KERN_INFO "READ DEVICE %s nbytes=%d ppos=%d\n", DEVICE_NAME, nbytes, (int)*ppos);
static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
    printk( KERN_INFO "foo_show foo_show_counter=%d\n", foo_show_counter);
    foo_show_counter++;
    printk( KERN_INFO "foo_show buf=%s\n", buf);
    return sprintf(buf, "%d\n", foo);
}

/*
 * foo_store дергается каждый раз когда я пишу в foo
 * buf -> данные которые пишу, count кол-во байт
 */

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count)
{
    int ret;
    printk(KERN_INFO "foo_store count=%d\n", count);
    //
    printk( KERN_INFO "foo_store foo_store_counter=%d\n", foo_store_counter);
    foo_store_counter++;
    //
    printk( KERN_INFO "foo_store buf=%s\n", buf);
    ret = kstrtoint(buf, 10, &foo);
    ret+=100;

    if (ret < 0)
        return ret;

    return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute foo_attribute =
        __ATTR(foo, 0664, foo_show, foo_store);

/*
 * В зависимости от того какой файл открываем
 * в attr->attr.name запишется baz или bar
 *
 */
static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf)
{
    printk( KERN_INFO "b_show baz_show_counter=%d\n", baz_show_counter);
    printk( KERN_INFO "b_show attr->attr.name=%s\n", attr->attr.name);

    int var;
    if (strcmp(attr->attr.name, "baz") == 0)
        var = baz;
    else
        var = bar;
    return sprintf(buf, "%d\n", var);
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr,
                       const char *buf, size_t count)
{
    int var, ret;

    ret = kstrtoint(buf, 10, &var);
    if (ret < 0)
        return ret;

    if (strcmp(attr->attr.name, "baz") == 0)
        baz = var;
    else
        bar = var;
    return count;
}

static struct kobj_attribute baz_attribute =
        __ATTR(baz, 0664, b_show, b_store);
static struct kobj_attribute bar_attribute =
        __ATTR(bar, 0664, b_show, b_store);


/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
        &foo_attribute.attr,
        &baz_attribute.attr,
        &bar_attribute.attr,
        NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
        .attrs = attrs,
};

static struct kobject *example_kobj;

static int __init example_init(void)
{
    int retval;

    /*
     * Create a simple kobject with the name of "kobject_example",
     * located under /sys/kernel/
     *
     * As this is a simple directory, no uevent will be sent to
     * userspace.  That is why this function should not be used for
     * any type of dynamic kobjects, where the name and number are
     * not known ahead of time.
     */
    example_kobj = kobject_create_and_add("kobject_example", kernel_kobj);
    if (!example_kobj)
        return -ENOMEM;

    /* Create the files associated with this kobject */
    retval = sysfs_create_group(example_kobj, &attr_group);
    if (retval)
        kobject_put(example_kobj);

    return retval;
}

static void __exit example_exit(void)
{
    kobject_put(example_kobj);
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Greg Kroah-Hartman <greg@kroah.com>");
