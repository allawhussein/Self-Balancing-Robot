#include <Wire.h>

float last_loop_time;
float gY_cal;
float angle;

float acc_angle()
{
        float acc, accX, accY, accZ;
        Wire.beginTransmission(0x68);
        Wire.write(0x3B);
        Wire.endTransmission();
        //toggle accelerometer register

        Wire.requestFrom(0x68, 6);
        accX = Wire.read()<<8||Wire.read();
        accY = Wire.read()<<8||Wire.read();
        accZ = Wire.read()<<8||Wire.read();
        Serial.print(accX);Serial.print(" ");Serial.print(accY);Serial.print(" ");Serial.println(accZ);

        acc = sqrt(accX*accX + accY*accY + accZ*accZ);

        return asin(accX/acc) * 180 / 3.14;
}

void calibrate_gY()
{
        float gY = 0;
        for (int i = 0; i < 1000; i++)
        {
                Wire.beginTransmission(0x68);
                Wire.write(0x45);//address of gyro_y register
                Wire.endTransmission();
                //toggle gyroscope register

                Wire.requestFrom(0x68, 2);
                gY += (Wire.read()<<8||Wire.read()) / 131;
        }

        gY_cal = gY / 1000;
}

float gyro_angle(float given)
{
        float gY;
        Wire.beginTransmission(0x68);
        Wire.write(0x45);//address of gyro_y register
        Wire.endTransmission();
        //toggle gyroscope register

        Wire.requestFrom(0x68, 2);
        gY = (Wire.read()<<8||Wire.read()) / 131;
        Serial.println(gY);
        given += (gY - gY_cal) * (last_loop_time - millis());
        last_loop_time = millis();
        return given;
}
void setup()
{
        Wire.begin();
        Wire.beginTransmission(0x68);//modify PWR_MGMT_1 to prevent device sleep or cycling
        Wire.write(0x6B);
        Wire.write(0b00000000);//7- reset/ 6- sleep/ 5- cycle (sleep & wake)/ 4- reserved/ 3- temp_dis/ 2,1,0- clock source
        Wire.endTransmission();

        Wire.beginTransmission(0x68);//modify scaling of gyroscope then accelerometer
        Wire.write(0x1B);
        Wire.write(0b00000000);//7- XG self test/6- YG self test/5- ZG self test/4,3- full scale/2,1,0 reserved
        Wire.endTransmission();
        Wire.beginTransmission(0x68);
        Wire.write(0x1C);
        Wire.write(0b00000000);//register descreption similar to gyroscope register
        Wire.endTransmission();
        Serial.begin(115200);
        Serial.println("calibrating");
        angle = acc_angle();
        calibrate_gY();
        last_loop_time = millis();
}

void loop()
{
        angle = 0.9 * gyro_angle(angle) + 0.1 * acc_angle();
        Serial.println(acc_angle());
        Serial.println(angle);
        Serial.println();
}