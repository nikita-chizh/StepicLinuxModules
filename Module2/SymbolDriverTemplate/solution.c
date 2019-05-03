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
// sudo mknod /dev/chrdrv c 700 0 && sudo chmod a+rw /dev/chrdrv
// ls -la /dev/chrdev
// sudo chmod a+rw /dev/chrdrv
// sudo echo "hi" > /dev/chrdrv

static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
// struct cdev
static struct cdev *my_cdev;

#define DEVICE_NAME "CHZH_EXAMPLE"
#define KBUF_SIZE 10*PAGE_SIZE


static int mydev_open(struct inode *pinode, struct file *pfile){
    printk( KERN_INFO "OPEN\n");
    printk( KERN_INFO "myopen %s KBUF_SIZE=%d\n", DEVICE_NAME, KBUF_SIZE);
    static int counter = 0;
    char *kbuf = kmalloc(KBUF_SIZE, GFP_KERNEL);
    pfile -> private_data = kbuf;
    printk( KERN_INFO "COUNTER=%d\n", counter);
    printk( KERN_INFO "REFERENCE COUNTER=%d\n", module_refcount(THIS_MODULE));

    return 0;
}

static int mydev_release(struct inode *pinode, struct file *pfile){
    printk( KERN_INFO "mydev_release\n", DEVICE_NAME);
    char *kbuf = pfile -> private_data;
    if(kbuf)
        kfree(kbuf);
    kbuf= NULL;
    pfile -> private_data = NULL;
    return 0;
}

static ssize_t mydev_read(struct file *pfile, char __user *buf, size_t lbuf, loff_t *ppos){
    char *kbuf = pfile -> private_data;
    static char test_msg[128] = "Hello world\0";
    memcpy(kbuf, test_msg, 12);
    int nbytes = lbuf - copy_to_user(buf, kbuf + *ppos, lbuf);
    printk( KERN_INFO "READ DEVICE %s nbytes=%d ppos=%d\n", DEVICE_NAME, nbytes, (int)*ppos);
    return nbytes;
}

static ssize_t mydev_write(struct file *pfile, const char __user *buf, size_t lbuf, loff_t *ppos){
    char *kbuf = pfile -> private_data;
    int nbytes = lbuf - copy_from_user(kbuf + *ppos, buf, lbuf);
    printk( KERN_INFO "WRITE DEVICE %s nbytes=%d ppos=%d\n", DEVICE_NAME, nbytes, (int)*ppos);
    return nbytes;
}

struct file_operations fops = {
        .owner = THIS_MODULE,
        .open = mydev_open,
        .read = mydev_read,
        .write = mydev_write,
        .release = mydev_release
};

static int __init hello_init(void) {
    printk( KERN_ALERT "Module started\n" ) ;
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
    if(my_cdev)
        cdev_del(my_cdev);
    unregister_chrdev_region(first, count);
}

module_init(hello_init) ;
module_exit(hello_exit) ;
