#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "checker.h"

MODULE_LICENSE("GPL") ;
MODULE_DESCRIPTION("This is a very important kernel module") ;

int __init init_module(void) {
    call_me( "Hello from my module" );
    return 0 ;
}

void __exit cleanup_module(void) {
}
