//#include <asm-generic/page.h>
//#include <asm-generic/uaccess.h>


#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/module.h>


// Шаблон драйвера символьного устройства

MODULE_AUTHOR("Chzh") ;
MODULE_LICENSE("GPL") ;
MODULE_DESCRIPTION("This is a very important kernel module");

static dev_t first;
static u_int32_t region_length = 1;
static int my_major = 700, my_minor = 0;

static struct cdev *my_cdev;

static struct class *my_class;
static char *node_name = "test";
module_param(node_name, charp, 0660);
static char help_buf[256];
static char was_readed = 0;

// sudo mknod /dev/solution_node c 700 0
// sudo chmod a+rw /dev/solution_node
// ls -la /dev/solution_node
// sudo chmod a+rw /dev/solution_node
// sudo echo "hi" > /dev/chrdrv


#define DEVICE_NAME "CHZH_EXAMPLE"
#define KBUF_SIZE 10 * PAGE_SIZE


static int mydev_open(struct inode *pinode, struct file *pfile){
    int nbytes = sprintf(help_buf, "OPEN major = %d pfile=%#llX pinode=%#llX\n", my_major,
            (unsigned long long)pfile, (unsigned long long)pinode);
    printk( KERN_INFO "%s\n", help_buf);
    return 0;
}

static int mydev_release(struct inode *pinode, struct file *pfile){
    int nbytes = sprintf(help_buf, "RELEASE major = %d pfile=%#llX\n", my_major,
                         (unsigned long long)pfile);
    printk( KERN_INFO "%s\n", help_buf);
    return 0;
}
//
static ssize_t mydev_read(struct file *pfile, char __user *buf, size_t read_size, loff_t *ppos){
    int nbytes = sprintf(help_buf, "READ major = %d pfile=%#llX\n", my_major, (unsigned long long)pfile);
    int copied = nbytes - copy_to_user(buf, help_buf, nbytes);
    if (!was_readed){
        was_readed = 1;
        return copied;
    }
    return 0;
}

struct file_operations fops = {
        .owner = THIS_MODULE,
        .open = mydev_open,
        .read = mydev_read,
        .release = mydev_release
};

static int __init hello_init(void) {
    // Отсюда берутся my_major my_minor
    if((alloc_chrdev_region(&first, 0, region_length, "Test_Dev")) <0){
        return -1;
    }
    my_major = MAJOR(first);
    my_minor = MINOR(first);
    //
    my_class = class_create(THIS_MODULE, "my_class");
    device_create(my_class, NULL, first, "%s", node_name);
    //
    first = MKDEV(my_major, my_minor);
    my_cdev = cdev_alloc();
    cdev_init(my_cdev, &fops);
    cdev_add(my_cdev, first, region_length);
    //
    return 0 ;
}

static void __exit hello_exit(void) {
    device_destroy(my_class, first);
    class_destroy(my_class);
    if(my_cdev)
        cdev_del(my_cdev);
    unregister_chrdev_region(first, region_length);
}

module_init(hello_init) ;
module_exit(hello_exit) ;