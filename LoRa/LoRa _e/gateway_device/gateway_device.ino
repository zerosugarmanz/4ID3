#include "gateway_device.h"

void setup()
{
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  Serial.println("connected");
  client.setServer(mqtt_server, 1883);//connecting to mqtt server

  Wire.begin();
  
  Serial_0.begin(9600, SERIAL_8N1, 16, 17);
  Transceiver.init();
  Transceiver.SetAddressH(1);
  Transceiver.SetAddressL(1);
  Transceiver.SetAirDataRate(ADR_9600);
  Transceiver.SetChannel(TRANSCEIVER_CHANNEL);
  Transceiver.SetMode(EBYTE_MODE_NORMAL);
  Transceiver.SetTransmitPower(OPT_TP20);
  Transceiver.SaveParameters(PERMANENT);
  Transceiver.PrintParameters();
  
  //  Initializing IO port for MCP23017 IO expansion bus
  mcp.init();
  mcp.portMode(MCP23017Port::A, 0); // Configuring port A as OUTPUT
  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Resetting port A 

}

void callback(char* topic, byte* payload, unsigned int length) {   
  Serial.println("Callback called");
  
  String data_str = "";
  String topic_str = "";
  for(int i = 0; topic[i] != '\0'; i ++){
    topic_str += (char)topic[i];
  }
  for (unsigned int i = 0; i < length; i++)
  {
    data_str += (char)payload[i];
  }
  Serial.println("Data received:\n   Topic: " + topic_str + "\n   Data: " + data_str + "\n");

    if (topic_str == (String(group_name) + "/" + field_device_name + "/LED2")){
      Serial.println("Forwarding: " + data_str);
      if(data_str == "0"){
        Serial2.println('2');
      }
      else if(data_str == "1"){
        Serial2.println('3');
      }
   }  
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!client.connected())
  {
      Serial.println("Reconnecting to MQTT broker");
      client.connect(String(String(group_name) + "_" + String(gateway_device_name)).c_str());
        client.setCallback(callback);
      if(client.state() == MQTT_CONNECTED){
        Serial.println("Connected to MQTT broker");
        client.subscribe(String(String(group_name) + "/" + field_device_name + "/LED2").c_str());
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        Serial.println("Wifi Signal Strength: " + String(WiFi.RSSI()) + " dB");
      }
      else{
        Serial.println("Failed to connect, rc = " + client.state());
      }
  }

  client.loop();

  if (Serial2.available() > 1) {
    Serial.println("Receiving Data...");
    String incoming_data = Serial2.readString();
    StaticJsonDocument<sizeof(incoming_data) + 200> json_document;
    DeserializationError error = deserializeJson(json_document, incoming_data.c_str());
    const char* field_device_temp = json_document[group_name][field_device_name]["Temp"];
    const char* field_device_lum = json_document[group_name][field_device_name]["Luminosity"];
    client.publish(String(String(group_name) + "/" + String(field_device_name) + "/Temp").c_str(), field_device_temp);
    client.publish(String(String(group_name) + "/" + String(field_device_name) + "/Luminosity").c_str(), field_device_lum);

    Serial.println("Field Device Light Intensity: " + String(String(field_device_lum).toFloat()));
    
    if(String(field_device_lum).toFloat() > 200.0){
      Serial.println("Telling field_device to actuate LED 1 ON");
      Serial2.println('1');
    }
    else{
      Serial.println("Telling field_device to actuate LED 1 OFF");
      Serial2.println('0');
    }
 }
}
