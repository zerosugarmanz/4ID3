#include "bluetooth_a.h"

void setup() { //Configure I2C temperature sensor
  Serial.begin(9600);
  Serial.print("\n\n------------------------\n"
    + group_name + " : " + device_name + "\n------------------------\n\n"); 

  Wire.begin(); //初始化 I²C 总线
  Wire.beginTransmission(ADDR); //开始与指定地址(ADDR)的从设备通信
  Wire.endTransmission();
  delay(300);
  
  light_sensor.begin(apds_gain, apds_time); //Establish connection to your light intensity sensor
  
}

void loop() { //Write the polling command to poll the temperature sensor into it’s I2C data register
  Wire.beginTransmission(ADDR);
  Wire.write(HUM_CMD); //写入一个命令（TMP_CMD），告诉传感器发送温度数据。
  Wire.endTransmission();
  delay(100);

  Wire.requestFrom(ADDR, 2); //向从设备请求2字节数据（通常用于读取温度或其他传感器数据）

  char data[2]; //Read 2 bytes from the I2C bus register
  if(Wire.available() == 2){
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  float humi = ((data[0] * 256.0) + data[1]);
  float humi_c = ((125 * humi) / 65536.0) - 6;

  AsyncAPDS9306Data light_data = light_sensor.syncLuminosityMeasurement(); // Poll the light sensor and convert it to comfortable units
  float lux = light_data.calculateLux();

//Print this data to the serial monito and format as a JSON
  String formatted_data = 
    "{ \"" + group_name + "\": { \"" 
    + device_name + "\": { \"Humidity\": \"" 
    + String(humi_c) + "%\", \"Luminosity\": \"" 
    + String(lux) + "\" } } }" + '\n';
      
  Serial.println(formatted_data);
  
  delay(DELAY_BETWEEN_SAMPLES_MS);

}
