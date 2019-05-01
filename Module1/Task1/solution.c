#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_AUTHOR("Chzh") ;
MODULE_LICENSE("GPL") ;
MODULE_DESCRIPTION("This is a very important kernel module") ;

int __init hello_init(void) {
    printk( KERN_ALERT "Module started\n" ) ;
    return 0 ;
}

void __exit hello_exit(void) {
    printk( KERN_ALERT "Module unloaded\n" ) ;
}

module_init(hello_init) ;
module_exit(hello_exit) ;
