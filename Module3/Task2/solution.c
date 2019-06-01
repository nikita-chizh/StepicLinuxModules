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

//
// Created by nikita on 02.02.17.
//
#define BUF_SIZE 256
struct Ring_Buffer{
    char data[BUF_SIZE];// данные
    char *cur_pos;//позиция
    char *end;
}Ring_Buffer;


struct Ring_Buffer *create_ring_buffer(void);

int size_counting(struct Ring_Buffer *buffer, size_t *expected_size, size_t *available_size);
// сколько записал столько и верну
size_t write_data(struct Ring_Buffer *buffer, __user char* data, size_t size) ;
//
size_t read_data(struct Ring_Buffer *buffer, __user char* data, size_t size) ;

loff_t pos_move(struct Ring_Buffer *buffer, loff_t offset, int whence);
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
    struct Ring_Buffer *buffer = create_ring_buffer();
    filp -> private_data = (void*)buffer;
    return 0;
}

static int mydev_release(struct inode *pinode, struct file *pfile){
    printk( KERN_INFO "mydev_release\n", DEVICE_NAME);
    struct Ring_Buffer *buffer = (struct Ring_Buffer*)pfile -> private_data;
    if(buffer)
        delete_buffer(buffer);
    pfile -> private_data = NULL;
    return 0;
}
//
static ssize_t mydev_read(struct file *pfile, char __user *buf, size_t read_size, loff_t *fpos){
    struct Ring_Buffer *buffer = (struct Ring_Buffer*)pfile -> private_data;
    if(!buffer)
        return 0;
    size_t nbytes = read_data(buffer, buf, read_size);
    fpos += nbytes;
    return nbytes;
}

static ssize_t mydev_write(struct file *pfile, const char __user *buf, size_t write_size, loff_t *fpos){
    struct Ring_Buffer *buffer = (struct Ring_Buffer*)pfile -> private_data;
    if(!buffer)
        return 0;
    size_t nbytes = write_data(buffer, buf, write_size);
    fpos += nbytes;
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
    struct Ring_Buffer *buffer = (struct Ring_Buffer*)pfile -> private_data;
    if(!buffer)
        printk( KERN_ERR "mydev_llseek buffer==NULL %s\n", DEVICE_NAME);
    else
        pos_move(buffer, offset, origin);
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
    struct Ring_Buffer *buffer_ptr
            = (struct Ring_Buffer*)kmalloc(sizeof(struct Ring_Buffer), GFP_ATOMIC);
    if (buffer_ptr == NULL) {
        return NULL;
    }
    buffer_ptr -> cur_pos = 0;
    buffer_ptr -> cur_pos = buffer_ptr->data;
    buffer_ptr -> end = buffer_ptr -> data + BUF_SIZE;
    return buffer_ptr;
}

int size_counting(struct Ring_Buffer *buffer, size_t *expected_size, size_t *available_size)
{
    size_t max_available_size = buffer->end - buffer->cur_pos;
    if (max_available_size < 0)
        return -1;
    if (*expected_size > max_available_size)
        *available_size = max_available_size;
    else
        *available_size = *expected_size;
    return 0;
}
// сколько записал столько и верну
size_t write_data(struct Ring_Buffer *buffer, __user char* data, size_t size) {
    if (buffer == NULL) {
        return 0;
    }
    size_t size_to_write = 0;
    int check_border = size_counting(buffer, &size, &size_to_write);
    if(check_border != 0)
        return 0;
    int nbytes = size - copy_from_user(buffer->cur_pos, data, size_to_write);
    buffer->cur_pos += nbytes;
    return nbytes;
}
//
size_t read_data(struct Ring_Buffer *buffer, __user char* data, size_t size) {
    if (buffer == NULL) {
        return 0;
    }
    size_t size_to_read = 0;
    int check_border = size_counting(buffer, &size, &size_to_read);
    if(check_border != 0)
        return 0;
    int nbytes = size - copy_to_user(data, buffer->cur_pos, size_to_read);
    buffer->cur_pos += nbytes;
    return nbytes;
}

loff_t pos_move(struct Ring_Buffer *buffer, loff_t offset, int whence)
{
    switch (whence) {
        case SEEK_SET:
            buffer->cur_pos = buffer->data + offset;
            break;
        case SEEK_CUR:
            buffer->cur_pos += offset;
            break;
        case SEEK_END:
            buffer->cur_pos = buffer->end + offset;
            break;
        default:
            return -EINVAL;
    }
    if(buffer->cur_pos > buffer->end)
        buffer->cur_pos = buffer->end - 1;
    if(buffer->cur_pos < buffer->data)
        buffer->cur_pos = buffer->data;
    return buffer->cur_pos;
}

//
int delete_buffer(struct Ring_Buffer *buffer) {
    if (buffer == NULL) {
        return -1;
    }
    kfree(buffer);
    buffer = NULL;
}
