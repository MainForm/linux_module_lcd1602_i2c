#include <linux/module.h>

#include "lcd1602.h"

const struct file_operations lcd1602_fops = {
    .owner = THIS_MODULE,
};

