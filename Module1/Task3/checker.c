#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>


MODULE_AUTHOR("Specialist <info@specialist.ru>") ;
MODULE_LICENSE("GPL") ;
MODULE_DESCRIPTION("This is a very important kernel module") ;

int __init init_module(void) {
    printk( KERN_ALERT "Module hello3_one started\n" ) ;
    return 0 ;
}

void __exit cleanup_module(void) {
    printk( KERN_ALERT "Module hello3_one unloaded\n" ) ;
}

int array_sum(short *arr, size_t n){
    int sum = 0;
    for(int i = 0; i < n; ++i)
        sum += arr[i];
    return sum;
}

void foo(){}

size_t generate_output(int sum, short *arr, size_t size, char *buf){
    return 0;
}

EXPORT_SYMBOL(foo);
EXPORT_SYMBOL(array_sum);
EXPORT_SYMBOL(generate_output);