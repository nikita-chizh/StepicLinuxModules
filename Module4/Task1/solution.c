#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
/*
 * Разработать модуль ядра solution, который обращается к связному списку
 * структуры struct module из заголовочного файла linux/module.h и
 * выводит имена всех модулей в kobject с именем /sys/kernel/my_kobject/my_sys.
 * Имена модулей должны выводиться в алфавитном порядке, каждое на новой строке.
 */
static int my_sys;

// show и strore, это функции которые дергаются когда файл в sysfs читается/пишется
// buf это буфер который вернется в uspace, пришел из uspace

static ssize_t my_show(struct kobject *kobj, struct kobj_attribute *attr,
                       char *buf)
{

    struct list_head *head = &THIS_MODULE->list;
    struct list_head *next = head->next;
    printk( KERN_INFO "HEAD ADDR == %#llX\n", head);
    printk( KERN_INFO "NEXT ADDR == %#llX\n", next);
    next = NULL;
    list_for_each(next, head) {
        struct module *currentobj = container_of(next, struct module, list);
        printk( KERN_INFO "next -> name == %s\n", currentobj -> name);
    }
    return sprintf(buf, "%d\n", 1);
}

static ssize_t my_store(struct kobject *kobj, struct kobj_attribute *attr,
                        const char *buf, size_t count)
{
    return count;
}

// ВНЕЗАПНО, файл будет создан с НАЗВАНИЕМ переменной my_sys
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
