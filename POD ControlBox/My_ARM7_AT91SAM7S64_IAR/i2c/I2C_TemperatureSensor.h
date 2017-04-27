#ifndef I2C_TEMPERATURESENSOR_H
#define I2C_TEMPERATURESENSOR_H

#define  SENSOR_1_I2C_INT_CB_ADRESS        0x49
#define  SENSOR_1_I2C_EXT_FALL_ADRESS      0x4A
#define  READ_TEMP_SENSOR_COMMAND_DS1621   0xAA
#define  READ_TEMP_SENSOR_COMMAND_LM75     0x00

#define I2C_TYPE_DS1621  0x01
#define I2C_TYPE_LM75    0x02

int GetTemperature(unsigned char* pTemperature, int * pTemperatureLength,  int nI2C_SensorID, char bDS1621);
//char TestSensorType(int nI2Caddress);
#endif
