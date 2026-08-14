#define pr_fmt(fmt) "[" KBUILD_MODNAME "][%s]: " fmt, __func__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

#define LCD1602_CHAR_DEV_NAME   "lcd1602"
#define LCD1602_CLASS_NAME      "char-lcd"


// 문자 장치 관련 변수
static int major;

struct class *lcd1602_class;
struct device *lcd1602_device; 

static const struct file_operations fops = {
    .owner = THIS_MODULE,
};

static int __init init_device(void){
    int ret = 0;
    
    // 문자 장치 등록

    major = register_chrdev(0, LCD1602_CHAR_DEV_NAME, &fops);
    if(major < 0){
        pr_err("register_chrdev() failed: %d\n",major);

        ret = major;
        goto err_chrdev;
    }

    pr_info("registered lcd character module\n");

    // 문자 장치를 sysfs에 등록
    // "/sys/class"에 char-lcd 생성
    lcd1602_class = class_create(LCD1602_CLASS_NAME);
    if(IS_ERR(lcd1602_class)){
        pr_err("class_create() failed: %pe\n", lcd1602_class);

        ret = PTR_ERR(lcd1602_class);
        goto err_unregister_chardev;
    }

    pr_info("created lcd character class\n");

    lcd1602_device = device_create(lcd1602_class,NULL, MKDEV(major,0),NULL,LCD1602_CHAR_DEV_NAME);
    if(IS_ERR(lcd1602_device)){
        pr_err("device_create() failed: %pe\n", lcd1602_device);

        ret = PTR_ERR(lcd1602_device);
        goto err_class_destroy;
    }

    pr_info("created lcd character device\n");

    return 0;


err_class_destroy:
    class_destroy(lcd1602_class);

err_unregister_chardev:
    unregister_chrdev(major,LCD1602_CHAR_DEV_NAME);
    
err_chrdev:
    return ret;
}

static void __exit exit_device(void){

    device_destroy(lcd1602_class, MKDEV(major, 0));

    class_destroy(lcd1602_class);

    unregister_chrdev(major,LCD1602_CHAR_DEV_NAME);

    pr_info("removed lcd module\n");
}

module_init(init_device);
module_exit(exit_device);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module is for LCD 1602 display with I2C");
