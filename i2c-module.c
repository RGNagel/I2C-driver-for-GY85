#include <linux/module.h>

static int __init init_i2c_client(void) {
    printk("hello world!\n");

    return 0;
}

static void __exit exit_i2c_client(void) {
    printk("bye bye world!\n");
}

module_init(init_i2c_client);
module_exit(exit_i2c_client);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafael Gustavo Nagel");
MODULE_DESCRIPTION("Module for reading i2c peripherals, namely ITG3205, ADXL345 and HMC5883L.");