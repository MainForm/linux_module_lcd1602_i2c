#define pr_fmt(fmt) "[" KBUILD_MODNAME "][%s]: " fmt, __func__

#include <linux/delay.h>

#include "lcd1602.h"

#define I2C_DEV_TYPE       	"lcd1602_i2c"

#define LCD1602_MODE_RS		(0x01 << 0)
#define LCD1602_MODE_RW		(0x01 << 1)
#define LCD1602_MODE_EN		(0x01 << 2)
#define LCD1602_MODE_BL		(0x01 << 3)		// back lgith

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

void lcd1602_send_data(struct lcd1602_device *lcd, const u8 data, const u8 mode){
	u8 value = ((data & 0x0f) << 4) | mode;

	u8 buffers[2] = {
		value | LCD1602_MODE_EN,
		value & ~LCD1602_MODE_EN,
	};

	i2c_master_send(lcd->client, buffers, sizeof(buffers) / sizeof(u8));
}

void lcd1602_send_command(struct lcd1602_device *lcd, const u8 command){
    u8 mode = LCD1602_MODE_BL;

    lcd1602_send_data(lcd, command >> 4, mode);
    lcd1602_send_data(lcd, command & 0x0f, mode);


    if (command == 0x01 || command == 0x02 || command == 0x03)
        fsleep(2000);
    else
        fsleep(50);
}

void lcd1602_send_char(struct lcd1602_device *lcd, const char character)
{
    u8 mode = LCD1602_MODE_BL | LCD1602_MODE_RS;

    lcd1602_send_data(lcd, character >> 4, mode);
    lcd1602_send_data(lcd, character & 0x0f, mode);

	fsleep(50);
}

void lcd1602_send_string(struct lcd1602_device *lcd, const char* str){
    while(*str){
        lcd1602_send_char(lcd,*str);
        str++;
    }
}

void lcd1602_init_device(struct lcd1602_device *lcd){

    /* 전원 안정화 */
    fsleep(50000);

    /* 아직 8비트 초기 상태이므로 니블만 전송 */
    lcd1602_send_data(lcd, 0x03, LCD1602_MODE_BL);
    fsleep(5000);

    lcd1602_send_data(lcd, 0x03, LCD1602_MODE_BL);
    fsleep(200);

    lcd1602_send_data(lcd, 0x03, LCD1602_MODE_BL);
    fsleep(200);

    /* 4비트 모드로 전환 */
    lcd1602_send_data(lcd, 0x02, LCD1602_MODE_BL);
    fsleep(200);

    /* 4비트, 2줄, 5x8 폰트 */
    lcd1602_send_command(lcd, 0x28);

    /* Display off */
    lcd1602_send_command(lcd, 0x08);

    /* Clear display */
    lcd1602_send_command(lcd, 0x01);

    /* Entry mode: 커서 오른쪽 이동 */
    lcd1602_send_command(lcd, 0x06);

    /* Display on, cursor off, blink off */
    lcd1602_send_command(lcd, 0x0c);
}