#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>

#include <bme680_reg.h>

#define BME680_ADDR     0x77

#define I2C_LABEL       i2c0



#define BME680_PART1_LSB 0xE9
#define BME680_PART1_MSB 0xEA
#define BME680_PART2_LSB 0x8A
#define BME680_PART2_MSB 0x8B
#define BME680_PART3    0x8C


const struct device *i2c_bus = DEVICE_DT_GET(DT_NODELABEL(I2C_LABEL));

int16_t part1;
int16_t part2;
int8_t part3;
int32_t t_fine;

void read_calibration_params() {
    uint8_t par_t1_lsb, par_t1_msb;
    uint8_t par_t2_lsb, par_t2_msb;
    uint8_t par_t3_val;

    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_PART1_LSB, &par_t1_lsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_PART1_MSB, &par_t1_msb);
    part1 = (par_t1_msb << 8) | par_t1_lsb;

    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_PART2_LSB, &par_t2_lsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_PART2_MSB, &par_t2_msb);
    part2 = (par_t2_msb << 8) | par_t2_lsb;

    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_PART3, &par_t3_val);
    part3 = par_t3_val;


}

float read_temp(){
    

    uint8_t temp_msb, temp_lsb, temp_xlsb;

    if (i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_MSB, &temp_msb) < 0 ||
        i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_LSB, &temp_lsb) < 0 ||
        i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_XLSB, &temp_xlsb) < 0) {
        printf("Failed to read temperature data from BME680.\n");
        return -1;
    }


    int32_t temp_adc = ((int32_t)temp_msb << 12) | ((int32_t)temp_lsb << 4) | ((int32_t)temp_xlsb >> 4);

    int32_t var1 = ((int32_t)temp_adc >> 3) - ((int32_t)part1 << 1);
    int32_t var2 = (var1 * (int32_t)part2) >> 11;
    int32_t var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((int32_t)part3 << 4)) >> 14;
    t_fine = var2 + var3;
    int32_t temp_comp = ((t_fine * 5) + 128) >> 8;

    float temp_celsius = temp_comp / 100.0;

    printf("Temperature: %.2f °C\n", temp_celsius);

    return temp_celsius;
}


int main(void)
{

    if (i2c_bus == NULL)
    {
        printf("No device found; did initialization fail?\n");
        return -1;
    } 
    
    /* Try and read chip id */
    uint8_t id;
    if (i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_ID, &id) < 0){
        printf("Could not communicate with sensor.\n"); 
        return -1; 
    }
    
    if (id != 0x61)
    {
        printf("Sensor ID could not be read from I2C device.\n");
        return -1;
    }


    
    printf("BME680 sensor found with ID: 0x%x\n", id);

    /* Initialize the BME680 sensor */
    uint8_t ctrl_meas = 0xB7; // Set oversampling and mode
    if (i2c_reg_write_byte(i2c_bus, BME680_ADDR, BME680_CTRL_MEAS, ctrl_meas) < 0) {
        printf("Failed to write to BME680_CTRL_MEAS register.\n");
        return -1;
    }

    /* Read temperature data */
    uint8_t temp_msb, temp_lsb, temp_xlsb;

    if (i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_MSB, &temp_msb) < 0 ||
        i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_LSB, &temp_lsb) < 0 ||
        i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_TEMP_XLSB, &temp_xlsb) < 0) {
        printf("Failed to read temperature data from BME680.\n");
        return -1;
    }

    /* Combine the temperature data */
    int32_t temp_raw = ((int32_t)temp_msb << 12) | ((int32_t)temp_lsb << 4) | ((int32_t)temp_xlsb >> 4);

    /* Convert raw temperature to degrees Celsius (using BME680 compensation formula) */
    // Note: The actual compensation formula is more complex and requires calibration data from the sensor.
    // Here we provide a simplified version for demonstration purposes.
    float temp_celsius = temp_raw / 100.0;

    read_calibration_params();

    printf("Temperature: %.2f °C\n", temp_celsius);
    while(1){
        read_temp();
        k_msleep(1000);
    }

    return 0;

}
