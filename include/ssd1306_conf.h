#ifndef SSD1306_CONF_H
#define SSD1306_CONF_H

/* Display configuration */
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64

/* I2C configuration */
#define SSD1306_I2C_ADDR    0x3C

/* ESP-IDF I2C configuration */
#define SSD1306_I2C_PORT     I2C_NUM_0
#define SSD1306_I2C_SDA      GPIO_NUM_8
#define SSD1306_I2C_SCL      GPIO_NUM_9
#define SSD1306_I2C_FREQ_HZ  400000

#endif /* SSD1306_CONF_H */