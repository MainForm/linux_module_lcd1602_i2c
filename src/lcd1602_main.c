#define pr_fmt(fmt) "[" KBUILD_MODNAME "][%s]: " fmt, __func__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

#include "lcd1602.h"

#define LCD1602_CHAR_DEV_NAME   "lcd1602"
#define LCD1602_CLASS_NAME      "char-lcd"

#define LCD1602_I2C_NR_DEFAULT          1
#define LCD1602_I2C_ADDRESS_DEFAULT     0x27

static struct lcd1602_device lcd_device;

struct lcd1602_device *lcd1602_get_device(void){
    return &lcd_device;
}

static int __init init_device(void){
    int ret = 0;
    
    atomic_set(&lcd_device.opened, 0);

    // 문자 장치 등록
    lcd_device.major = register_chrdev(0, LCD1602_CHAR_DEV_NAME, &lcd1602_fops);
    if(lcd_device.major < 0){
        pr_err("register_chrdev() failed: %d\n",lcd_device.major);

        ret = lcd_device.major;
        goto err_chrdev;
    }

    pr_info("registered lcd character module\n");

    // 문자 장치를 sysfs에 등록
    // "/sys/class"에 char-lcd 생성
    lcd_device.class = class_create(LCD1602_CLASS_NAME);
    if(IS_ERR(lcd_device.class)){
        pr_err("class_create() failed: %pe\n", lcd_device.class);

        ret = PTR_ERR(lcd_device.class);
        goto err_unregister_chardev;
    }

    pr_info("getting i2c client\n");
    // i2c client 획득
    // 디스플레이와 I2C 통신 확인
    ret = lcd1602_i2c_init(&lcd_device, LCD1602_I2C_NR_DEFAULT, LCD1602_I2C_ADDRESS_DEFAULT);

    pr_info("created lcd character class\n");

    // /dev 폴더에 장치 파일 생성
    lcd_device.device = device_create(lcd_device.class,NULL, MKDEV(lcd_device.major,0),NULL,LCD1602_CHAR_DEV_NAME);
    if(IS_ERR(lcd_device.device)){
        pr_err("device_create() failed: %pe\n", lcd_device.device);

        ret = PTR_ERR(lcd_device.device);
        goto err_class_destroy;
    }

    pr_info("created lcd character device\n");

    if(ret){
        pr_err("lcd1602_i2c_init() failed\n");
        goto err_i2c_exit;
    }

    pr_info("success to get i2c client\n");

    return 0;

err_i2c_exit:
    lcd1602_i2c_exit(&lcd_device);

err_class_destroy:
    class_destroy(lcd_device.class);

err_unregister_chardev:
    unregister_chrdev(lcd_device.major,LCD1602_CHAR_DEV_NAME);
    
err_chrdev:
    return ret;
}

static void __exit exit_device(void){
    device_destroy(lcd_device.class, MKDEV(lcd_device.major, 0));

    lcd1602_i2c_exit(&lcd_device);

    class_destroy(lcd_device.class);

    unregister_chrdev(lcd_device.major,LCD1602_CHAR_DEV_NAME);

    pr_info("removed lcd module\n");
}

module_init(init_device);
module_exit(exit_device);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module is for LCD 1602 display with I2C");
