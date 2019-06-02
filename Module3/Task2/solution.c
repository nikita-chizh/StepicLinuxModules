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
// sudo chmod a+rw /dev/solution_node
// sudo echo "hi" > /dev/chrdrv

//
// Created by nikita on 02.02.17.
//
#define BUF_SIZE 257
struct Ring_Buffer{
    char data[BUF_SIZE];// данные
    char *cur_pos;//конечная позиция
    char *end;
}Ring_Buffer;

static int session_counter = 0;

struct Ring_Buffer *create_ring_buffer(void);

int size_counting(struct Ring_Buffer *buffer, const size_t *expected_size, size_t *available_size);
// сколько записал столько и верну
size_t write_data(struct Ring_Buffer *buffer, const __user char* userbuf, size_t size) ;
//
size_t read_data(struct Ring_Buffer *buffer, __user char* userbuf, size_t size) ;

int pos_move(struct Ring_Buffer *buffer, loff_t offset, int whence);
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
    size_t nbytes = 0;
    buffer = (struct Ring_Buffer*)pfile -> private_data;
    if(!buffer)
        return nbytes;
    nbytes = read_data(buffer, buf, read_size);
    // fpos += nbytes;
    return nbytes;
}

static ssize_t mydev_write(struct file *pfile, const char __user *buf, size_t write_size, loff_t *fpos){
    struct Ring_Buffer *buffer = NULL;
    size_t nbytes = 0;
    buffer = (struct Ring_Buffer*)(pfile -> private_data);
    if(!buffer)
        return 0;
    nbytes = write_data(buffer, buf, write_size);
    fpos += nbytes;
    return nbytes;

}

static loff_t mydev_llseek(struct file *pfile, loff_t offset, int origin){
    loff_t testpos;
    struct Ring_Buffer *buffer = NULL;
    buffer = (struct Ring_Buffer*)pfile -> private_data;
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
    struct Ring_Buffer *buffer_ptr = (struct Ring_Buffer*)kmalloc(
            sizeof(struct Ring_Buffer), GFP_ATOMIC);
    int written = 0;
    if (buffer_ptr == NULL) {
        return NULL;
    }
    buffer_ptr -> cur_pos = buffer_ptr->data;
    written = sprintf(buffer_ptr -> cur_pos, "%d", session_counter);
    buffer_ptr -> cur_pos += written;
    *(buffer_ptr -> cur_pos) = '\0';
    buffer_ptr -> end = buffer_ptr -> data + BUF_SIZE - 1;
    *(buffer_ptr -> end) = '\0';
    return buffer_ptr;
}

int size_counting(struct Ring_Buffer *buffer, const size_t *expected_size, size_t *available_size)
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
size_t write_data(struct Ring_Buffer *buffer, const __user char* userbuf, size_t size) {
    size_t size_to_write = 0;
    int check_border = 0;
    int nbytes = 0;
    if (buffer == NULL) {
        return 0;
    }
    check_border = size_counting(buffer, &size, &size_to_write);
    if(check_border != 0)
        return 0;
    nbytes = size_to_write - copy_from_user(buffer->cur_pos, userbuf, size_to_write);
    buffer->cur_pos += nbytes;
    *(buffer->cur_pos) = '\0';
    printk( KERN_ALERT "write_data BUFFER=%s\n", buffer->data);
    return nbytes;
}
//
size_t read_data(struct Ring_Buffer *buffer, __user char* userbuf, size_t size) {
    size_t size_to_read = 0;
    int check_border = 0;
    size_t nbytes = 0;
    if (buffer == NULL) {
        return nbytes;
    }
    check_border = size_counting(buffer, &size, &size_to_read);
    if(check_border != 0)
        return 0;
    printk( KERN_ALERT "read_data BUFFER=%s\n", buffer->data);
    copy_to_user(userbuf, buffer->data, size_to_read);
    nbytes = buffer->cur_pos - buffer->data;
    printk( KERN_ALERT "read_data nbytes=%ld\n", nbytes);
    return nbytes;
}
// %ld\n
int pos_move(struct Ring_Buffer *buffer, loff_t offset, int whence)
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
    if(buffer->cur_pos >= buffer->end)
        buffer->cur_pos = buffer->end - 1;
    if(buffer->cur_pos < buffer->data)
        buffer->cur_pos = buffer->data;
    *(buffer->cur_pos + 1) = '\0';
    return 0;
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
