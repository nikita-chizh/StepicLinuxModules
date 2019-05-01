#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

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

void call_me( char *msg ) {
    printk( KERN_ALERT "External message: %s\n", msg ) ;
}

EXPORT_SYMBOL(call_me) ;
