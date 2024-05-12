#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#define _REG_CONFIG                 0x00
#define _REG_SHUNTVOLTAGE           0x01
#define _REG_BUSVOLTAGE             0x02
#define _REG_POWER                  0x03
#define _REG_CURRENT                0x04
#define _REG_CALIBRATION            0x05

#define RANGE_16V                  0x00
#define RANGE_32V                  0x01

#define DIV_1_40MV                 0x00
#define DIV_2_80MV                 0x01
#define DIV_4_160MV                0x02
#define DIV_8_320MV                0x03

#define ADCRES_9BIT_1S            0x00
#define ADCRES_10BIT_1S           0x01
#define ADCRES_11BIT_1S           0x02
#define ADCRES_12BIT_1S           0x03
#define ADCRES_12BIT_2S           0x09
#define ADCRES_12BIT_4S           0x0A
#define ADCRES_12BIT_8S           0x0B
#define ADCRES_12BIT_16S          0x0C
#define ADCRES_12BIT_32S          0x0D
#define ADCRES_12BIT_64S          0x0E
#define ADCRES_12BIT_128S         0x0F

#define POWERDOW                  0x00
#define SVOLT_TRIGGERED           0x01
#define BVOLT_TRIGGERED           0x02
#define SANDBVOLT_TRIGGERED       0x03
#define ADCOFF                    0x04
#define SVOLT_CONTINUOUS          0x05
#define BVOLT_CONTINUOUS          0x06
#define SANDBVOLT_CONTINUOUS      0x07

#define INA219_I2C_ADDR            0x42

int ina219_i2c_fd;

int ina219_write(uint8_t reg, uint16_t data) {
  uint8_t buf[2];
  buf[0] = (data >> 8) & 0xFF;
  buf[1] = data & 0xFF;
  return write(ina219_i2c_fd, buf, 2);
}

int ina219_read(uint8_t reg, uint16_t *data) {
  uint8_t buf[2];
  if (write(ina219_i2c_fd, &reg, 1) != 1) {
    return -1;
  }
  if (read(ina219_i2c_fd, buf, 2) != 2) {
    return -1;
  }
  *data = (buf[0] << 8) | buf[1];
  return 0;
}

void ina219_init(void) {
  ina219_i2c_fd = open("/dev/i2c-1", O_RDWR);
  if (ina219_i2c_fd < 0) {
    perror("Failed to open I2C device");
    exit(1);
  }
  if (ioctl(ina219_i2c_fd, I2C_SLAVE, INA219_I2C_ADDR) < 0) {
    perror("Failed to set I2C slave address");
    exit(1);
  }

  uint16_t cal_value = 4096;
  ina219_write(_REG_CALIBRATION, cal_value);

  uint16_t config = RANGE_32V << 13 |
                     DIV_8_320MV << 11 |
                     ADCRES_12BIT_32S << 7 |
                     ADCRES_12BIT_32S << 3 |
                     SANDBVOLT_CONTINUOUS;
  ina219_write(_REG_CONFIG, config);
}

float ina219_get_shunt_voltage_mv(void) {
  uint16_t data;
  ina219_read(_REG_SHUNTVOLTAGE, &data);
  return (float)data * 0.01;
}

float ina219_get_bus_voltage_v(void) {
  uint16_t data;
  ina219_read(_REG_BUSVOLTAGE, &data);
  return (float)(data >> 3) * 0.004;
}

float ina219_get_current_ma(void) {
  uint16_t data;
  ina219_read(_REG_CURRENT, &data);
  float current = (float)data * 0.1;
  if (data & 0x8000) {
    current = -current;
  }
  return current;
}

float ina219_get_power_w(void) {
  uint16_t data;
  ina219_read(_REG_POWER, &data);
  float power = (float)data * 0.002;
  if (data & 0x8000) {
    power = -power;
  }
  return power;
}

int main(void) {
  ina219_init();
  float bus_voltage, shunt_voltage, current, power, p;

  while (1) {
    bus_voltage = ina219_get_bus_voltage_v();
    shunt_voltage = ina219_get_shunt_voltage_mv() / 1000.0;
    current = ina219_get_current_ma();
    power = ina219_get_power_w();
    p = (bus_voltage - 6) / 2.4 * 100;
    if (p > 100) p = 100;
    if (p < 0) p = 0;

    printf("Load Voltage:  %6.3f V\n", bus_voltage);
    printf("Current:       %9.6f A\n", current / 1000);
    printf("Power:         %6.3f W\n", power);
    printf("Percent:       %3.1f%%\n", p);
    printf("\n");

    sleep(2);
  }

  close(ina219_i2c_fd);
  return 0;
}
