#define pr_fmt(fmt) "[" KBUILD_MODNAME "][%s]: " fmt, __func__

#include "lcd1602.h"

#define I2C_DEV_TYPE       "lcd1602_i2c"

int lcd1602_i2c_init(struct lcd1602_device *lcd, int i2c_nr, int i2c_address){
    int ret = 0;

    struct i2c_board_info board_info = {
		I2C_BOARD_INFO(I2C_DEV_TYPE, i2c_address)
	};

    pr_info("getting i2c(nr : %d) adapter\n", i2c_nr);
    lcd->adapter = i2c_get_adapter(i2c_nr);
	if (!lcd->adapter) {
		pr_err("I2C adapter(nr : %d)not found\n", i2c_nr);
		return -ENODEV;
	}

	pr_info("checking for plain I2C transfer support\n");
	if (!i2c_check_functionality(lcd->adapter, I2C_FUNC_I2C)) {
		pr_err("I2C adapter(nr : %d) does not support plain I2C transfers\n", i2c_nr);
		ret = -EOPNOTSUPP;

		goto err_i2c_put_adapter;
	}

    pr_info("successed to get i2c adapter\n");

    pr_info("getting i2c(address : 0x%02x) client\n",i2c_address);
	lcd->client = i2c_new_client_device(lcd->adapter, &board_info);
	if (IS_ERR(lcd->client)) {
		pr_err("failed to register I2C device: %d\n", ret);
		lcd->client = NULL;
		ret = PTR_ERR(lcd->client);

        goto err_i2c_put_adapter;
	}

    return 0;


err_i2c_put_adapter:
	if (lcd->adapter){
		i2c_put_adapter(lcd->adapter);
        lcd->adapter = NULL;
    }

	return ret;
}


void lcd1602_i2c_exit(struct lcd1602_device *lcd){
	if (lcd->client) {
		i2c_unregister_device(lcd->client);
        lcd->client = NULL;
	}

	if (lcd->adapter){
		i2c_put_adapter(lcd->adapter);
        lcd->adapter = NULL;
    }
}