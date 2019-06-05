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
#include <linux/device.h>


// Шаблон драйвера символьного устройства

MODULE_AUTHOR("Chzh") ;
MODULE_LICENSE("GPL") ;
MODULE_DESCRIPTION("This is a very important kernel module");
// sudo mknod /dev/solution_node c 240 0
// sudo chmod a+rw /dev/solution_node
// ls -la /dev/solution_node
// sudo echo "hi" > /dev/chrdrv

//
// Created by nikita on 02.02.17.
//
#define BUF_SIZE 257
struct Ring_Buffer{
    char data[BUF_SIZE];// данные
    size_t cur_ind;//текущий индекс, суть конец файла
}Ring_Buffer;

static int session_counter = 0;
static const char ending = '\0';

struct Ring_Buffer *create_ring_buffer(void);
// сколько записал столько и верну
size_t write_data(struct Ring_Buffer *buffer, const __user char* userbuf, size_t size) ;
//
size_t read_data(struct Ring_Buffer *buffer, __user char* userbuf, size_t size) ;

size_t pos_move(struct Ring_Buffer *buffer, loff_t offset, int whence);
//
int delete_buffer(struct Ring_Buffer *buffer);

/*НЕПОСРЕДСТВЕННО КОД ДРАЙВЕРА*/
static dev_t dev_num;
static unsigned int count = 1;
// my_major это тип устройства (usb например) my_minor конкретное уст-во(например порт)
static int my_major = 240, my_minor = 0;
// struct cdev
static struct cdev *my_cdev;

static struct class *my_class;


#define DEVICE_NAME "CHZH_EXAMPLE"
#define KBUF_SIZE 10 * PAGE_SIZE


static int mydev_open(struct inode *inode, struct file *filp){
    struct Ring_Buffer *buffer = NULL;
    printk( KERN_INFO "mydev_open %s\n", DEVICE_NAME);
    buffer = create_ring_buffer();
    ++session_counter;
    filp -> private_data = (void*)buffer;
    return 0;
}

static int mydev_release(struct inode *pinode, struct file *pfile){
    struct Ring_Buffer *buffer = NULL;
    buffer = (struct Ring_Buffer*)pfile -> private_data;
    printk( KERN_INFO "mydev_release %s\n", DEVICE_NAME);
    if(buffer)
        delete_buffer(buffer);
    pfile -> private_data = NULL;
    return 0;
}
//
static ssize_t mydev_read(struct file *pfile, char __user *buf, size_t read_size, loff_t *fpos){
    struct Ring_Buffer *buffer = NULL;
    buffer = (struct Ring_Buffer*)pfile -> private_data;
    if(!buffer)
        return 0;
    return read_data(buffer, buf, read_size);
}

static ssize_t mydev_write(struct file *pfile, const char __user *buf, size_t write_size, loff_t *fpos){
    struct Ring_Buffer *buffer = NULL;
    size_t nbytes = 0;
    buffer = (struct Ring_Buffer*)(pfile -> private_data);
    if(!buffer)
        return 0;
    return write_data(buffer, buf, write_size);;

}

static loff_t mydev_llseek(struct file *pfile, loff_t offset, int origin){
    struct Ring_Buffer *buffer = NULL;
    buffer = (struct Ring_Buffer*)pfile -> private_data;
    if(!buffer)
    {
        printk( KERN_ERR "mydev_llseek buffer==NULL %s\n", DEVICE_NAME);
        return 0;
    }
    else
        return pos_move(buffer, offset, origin);
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
    dev_num = MKDEV(my_major, my_minor);
    register_chrdev_region(dev_num, count, DEVICE_NAME);
    my_cdev = cdev_alloc();
    cdev_init(my_cdev, &fops);
    cdev_add(my_cdev, dev_num, count);
    //
    my_class = class_create(THIS_MODULE, "my_class");
    device_create(my_class, NULL, dev_num, "%s", "my_chardev");
    printk( KERN_INFO "CREATED DEVICE class=%s\n", DEVICE_NAME);
    return 0 ;
}

static void __exit hello_exit(void) {
    printk( KERN_ALERT "Module unloaded\n" );
    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    if(my_cdev)
        cdev_del(my_cdev);
    unregister_chrdev_region(dev_num, count);
}

module_init(hello_init) ;
module_exit(hello_exit) ;




struct Ring_Buffer *create_ring_buffer(void) {
    struct Ring_Buffer *buffer_ptr = (struct Ring_Buffer*)kmalloc(
            sizeof(struct Ring_Buffer), GFP_ATOMIC);
    int written = 0;
    if (buffer_ptr == NULL) {
        return NULL;
    }
    written = sprintf(buffer_ptr -> data, "%d", session_counter);
    buffer_ptr -> cur_ind = written;
    return buffer_ptr;
}
// сколько записал столько и верну
size_t write_data(struct Ring_Buffer *buffer, const __user char* userbuf, size_t size) {
    int nbytes = 0;
    size_t av_size = 0;
    size_t size_to_write = size;
    if (buffer == NULL) {
        printk( KERN_ERR "write_data BUFFER=NULL\n" );
        return 0;
    }
    av_size = BUF_SIZE - buffer->cur_ind;
    if(size > av_size) {
        size_to_write = av_size;
    }
    nbytes = size_to_write - copy_from_user(&buffer->data[buffer->cur_ind], userbuf, size_to_write);
    buffer->cur_ind += nbytes;
    printk( KERN_ALERT "write_data BUFFER=%s\n", buffer->data);
    return nbytes;
}
//
size_t read_data(struct Ring_Buffer *buffer, __user char* userbuf, size_t size) {
    int nbytes = 0;
    size_t size_to_read = size;
    printk( KERN_ALERT "read_data0 size_to_read=%lld\n", size_to_read);

    if (buffer == NULL) {
        printk( KERN_ERR "read_data BUFFER=NULL\n" );
        return 0;
    }
    if(size_to_read >  buffer->cur_ind)
        size_to_read = buffer->cur_ind;
    printk( KERN_ALERT "read_data1 size_to_read=%ld\n", size_to_read);
    printk( KERN_ALERT "read_data BUFFER=%s\n", buffer->data);
    nbytes = size_to_read - copy_to_user(userbuf, buffer->data, size_to_read);
    buffer->cur_ind -= nbytes;
    printk( KERN_ALERT "read_data nbytes=%lld\n", nbytes);
    return nbytes;
}
// %ld\n
size_t pos_move(struct Ring_Buffer *buffer, loff_t offset, int whence)
{
    size_t size_to_end = BUF_SIZE - buffer->cur_ind;
    size_t size_to_begin = BUF_SIZE - size_to_end;

    switch (whence) {
        case SEEK_SET:
        {
            if(offset < 0){
                buffer->cur_ind = 0;
                break;
            }
            if(offset >= BUF_SIZE)
                buffer->cur_ind = BUF_SIZE - 1;
            else
                buffer->cur_ind = offset;
            break;
        }
        case SEEK_CUR:
        {
            if(offset < 0 && offset * -1 > size_to_begin){
                buffer->cur_ind = 0;
                break;
            }
            if(offset > 0 && offset > size_to_end){
                buffer->cur_ind = BUF_SIZE - 1;
                break;
            }
            buffer->cur_ind += offset;
            break;
        }
        case SEEK_END:
        {
            if(offset > 0){
                printk( KERN_ALERT "POSITIVE OFFSET WITH SEEK_END");
                break;
            }
            if(offset * -1 > size_to_begin){
                buffer->cur_ind = 0;
                break;
            }
            buffer->cur_ind += offset;
            break;
        }
        default:
            return -EINVAL;
    }
    return buffer->cur_ind;
}

//
int delete_buffer(struct Ring_Buffer *buffer) {
    if (buffer == NULL) {
        return -1;
    }
    kfree(buffer);
    buffer = NULL;
    return 0;
}
