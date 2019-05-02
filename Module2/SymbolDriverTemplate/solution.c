#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/random.h>
// Шаблон драйвера символьного устройства

MODULE_AUTHOR("Chzh") ;
MODULE_LICENSE("GPL") ;
MODULE_DESCRIPTION("This is a very important kernel module");

static char *kbuf;
// typedef  unsigned int    __u32;
// typedef  __u32           __kernel_dev_t;
// typedef  __kernel_dev_t  dev_t;
//
static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
// struct cdev
static struct cdev *my_cdev;

#define DEVICE_NAME "CHZH_EXAMPLE"
#define KBUF_SIZE 10*PAGE_SIZE


static int mydev_open(struct inode *pinode, struct file *pfile){
    printk( KERN_INFO "myopen %s\n", DEVICE_NAME);
    return 0;
}

static int mydev_release(struct inode *pinode, struct file *pfile){
    printk( KERN_INFO "mydev_release\n", DEVICE_NAME);
    return 0;
}

static ssize_t mydev_read(struct file *pfile, char __user *buf, size_t lbuf, loff_t *ppos){
printk( KERN_INFO "mydev_read\n", DEVICE_NAME);
return 0;
}

static ssize_t mydev_write(struct file *pfile, const char __user *buf, size_t lbuf, loff_t *ppos){
printk( KERN_INFO "mydev_write\n", DEVICE_NAME);
return 0;
}

struct file_operations fops = {
        .owner = THIS_MODULE,
        .read = mydev_read,
        .write = mydev_write,
        .release = mydev_release
};

static int __init hello_init(void) {
    printk( KERN_ALERT "Module started\n" ) ;
    kbuf = (char*)kmalloc(KBUF_SIZE, GFP_KERNEL);
    //
    first = MKDEV(my_major, my_minor);
    register_chrdev_region(first, count, DEVICE_NAME);
    my_cdev = cdev_alloc();
    cdev_init(my_cdev, &fops);
    cdev_add(my_cdev, first, count);
    return 0 ;
}

static void __exit hello_exit(void) {
    printk( KERN_ALERT "Module unloaded\n" ) ;
}

module_init(hello_init) ;
module_exit(hello_exit) ;
