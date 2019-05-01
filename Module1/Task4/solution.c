#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/device.h>
#include "checker.h"

MODULE_LICENSE("GPL") ;
MODULE_DESCRIPTION("This is a very important kernel module") ;

////**********************////
//ssize_t get_void_size(void)
//{
//    //ssize_t size_of_array;
//    //get_random_bytes ( &size_of_array, sizeof (size_of_array) );
//    return 128;
//}

//void submit_void_ptr(void *p)
//{
//    if(p)
//        printk(KERN_INFO "kernel_mooc submit_void_ptr p is OK");
//    else
//    printk(KERN_ERR "kernel_mooc submit_void_ptr p is OK");
//}

////**********************////
//ssize_t get_int_array_size(void)
//{
//    ssize_t res = get_void_size();
//    return res;
//}

//void submit_int_array_ptr(int *p)
//{
//    if(p)
//        printk(KERN_INFO "kernel_mooc submit_int_array_ptr p is OK");
//    else
//    printk(KERN_ERR "kernel_mooc submit_int_array_ptr p is OK");
//}
////**********************////

//void submit_struct_ptr(struct device *p)
//{
//    if(p)
//        printk(KERN_INFO "kernel_mooc submit_struct_ptr p is OK");
//    else
//    printk(KERN_ERR "kernel_mooc submit_struct_ptr p is OK");
//}

//void checker_kfree(void *p)
//{
//    if(p)
//        kfree(p);
//}



void *pVoid;
int *pInt;
struct device *pDevice;

int __init init_module(void) {
    ssize_t pVoidSize = get_void_size();
    pVoid = kmalloc(pVoidSize, GFP_ATOMIC);
    submit_void_ptr(pVoid);
    //
    ssize_t pIntSize = get_int_array_size();
    pInt = (int*)kmalloc(pIntSize*sizeof(int), GFP_ATOMIC);
    submit_int_array_ptr(pInt);
    //
    pDevice = (struct device*)kmalloc(sizeof(struct device), GFP_ATOMIC);
    submit_struct_ptr(pDevice);

    return 0 ;
}

void exit_module(void) {
    checker_kfree(pVoid);
    checker_kfree((void*)pInt);
    checker_kfree((void*)pDevice);
}
module_exit(exit_module);