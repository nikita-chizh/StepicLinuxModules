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

#include <linux/module.h>


// Шаблон драйвера символьного устройства

MODULE_AUTHOR("Chzh") ;
MODULE_LICENSE("GPL") ;
MODULE_DESCRIPTION("This is a very important kernel module");
// sudo mknod /dev/solution_node c 700 0
// sudo chmod a+rw /dev/solution_node
// ls -la /dev/solution_node
// sudo chmod a+rw /dev/solution_node
// sudo echo "hi" > /dev/chrdrv

static dev_t first;
static unsigned int count = 1;
// my_major это тип устройства (usb например) my_minor конкретное уст-во(например порт)
static int my_major = 700, my_minor = 0;
// struct cdev
static struct cdev *my_cdev;

static struct class *my_class;


#define DEVICE_NAME "CHZH_EXAMPLE"
#define KBUF_SIZE 10 * PAGE_SIZE


static int mydev_open(struct inode *inode, struct file *filp){
    struct my_dev_data  *dev_data;

    dev_data = container_of(inode->i_cdev, struct my_dev_data, cdev);

    /* Allocate memory for file data and channel data */
    file_data = devm_kzalloc(&dev_data->pdev->dev,
                             sizeof(struct file_data), GFP_KERNEL);

    /* Add open file data to list */
    INIT_LIST_HEAD(&file_data->file_open_list);
    list_add(&file_data->file_open_list, &dev_data->file_open_list);

    file_data->dev_data = dev_data;
    filp->private_data = file_data;

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
//
static ssize_t mydev_read(struct file *pfile, char __user *buf, size_t read_size, loff_t *ppos){
    char *kbuf = pfile -> private_data;
    // head -8 /dev/chrdrv
    static char test_msg[128] = "Hello world";
    memcpy(kbuf, test_msg, 12);
    int nbytes = read_size - copy_to_user(buf, kbuf + *ppos, read_size);
    // ppos это позиция в  kbuf, ее надо двигать самостоятельно
    printk( KERN_INFO "READ DEVICE %s nbytes=%d ppos=%d\n", DEVICE_NAME, nbytes, (int)*ppos);
    return nbytes;
}

static ssize_t mydev_write(struct file *pfile, const char __user *buf, size_t lbuf, loff_t *ppos){
    char *kbuf = pfile -> private_data;
    int nbytes = lbuf - copy_from_user(kbuf + *ppos, buf, lbuf);
    printk( KERN_INFO "WRITE DEVICE %s nbytes=%d ppos=%d\n", DEVICE_NAME, nbytes, (int)*ppos);
    return nbytes;
}

static loff_t mydev_llseek(struct file *pfile, loff_t offset, int origin){
    loff_t testpos;
    switch (origin){
        case SEEK_SET:
            testpos = offset;
            break;
        case SEEK_CUR:
            testpos = pfile->f_pos + offset;
            break;
        case SEEK_END:
            testpos = KBUF_SIZE + offset;
            break;
        default:
            return -EINVAL;
    }
    if (testpos > KBUF_SIZE)
        testpos = KBUF_SIZE;
    else if (testpos < KBUF_SIZE)
        testpos = 0;
    pfile->f_pos = testpos;
    printk( KERN_INFO "mydev_llseek = %s\n", testpos);
    return testpos;
}

struct file_operations fops = {
        .owner = THIS_MODULE,
        .open = mydev_open,
        .read = mydev_read,
        .write = mydev_write,
        .release = mydev_release,
        .llseek = mydev_llseek
};

static int __init hello_init(void) {
    printk( KERN_ALERT "Module started\n" ) ;
    //
    first = MKDEV(my_major, my_minor);
    register_chrdev_region(first, count, DEVICE_NAME);
    my_cdev = cdev_alloc();
    cdev_init(my_cdev, &fops);
    cdev_add(my_cdev, first, count);
    //
    my_class = class_create(THIS_MODULE, "my_class");
    device_create(my_class, NULL, first, "%s", "my_chardev");
    printk( KERN_INFO "CREATED DEVICE class=%s\n", DEVICE_NAME);
    return 0 ;
}

static void __exit hello_exit(void) {
    printk( KERN_ALERT "Module unloaded\n" );
    device_destroy(my_class, first);
    class_destroy(my_class);
    if(my_cdev)
        cdev_del(my_cdev);
    unregister_chrdev_region(first, count);
}

module_init(hello_init) ;
module_exit(hello_exit) ;
