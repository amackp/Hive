#include <HX711.h>

#include <DHT.h>
#include <DHT_U.h>
#include  "Hive.h"

#define SOUND_PIN   A0
#define DHTPIN      4
#define DHTTYPE     DHT11   
#define PELTIER_PIN 6

#define TEMP_HYSTERYSIS 3

DHT_Unified dht(DHTPIN, DHTTYPE);

HiveData hive_data;

const int LOADCELL_DOUT_PIN = 5;
const int LOADCELL_SCK_PIN = 6;

#define SCALE_CAL_VALUE -7000.f 

HX711 scale;

uint32_t delayMS;

void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

void hive_push_data(){
  
    String payload = HIVE_INSIDE_TEMP + String(hive_data.inside_temperature_c)+",";
    //String cmd = "AT+SEND=501,"+String(payload.length()) +","+ payload;
    //lora_send_cmd(cmd, NULL, 1);
    //delay(500);

    payload += HIVE_INSIDE_HUMID + String(hive_data.inside_humidity)+",";
    //cmd = "AT+SEND=501,"+String(payload.length()) +","+ payload;
    //lora_send_cmd(cmd, NULL, 1);
    //delay(500);

    payload += HIVE_OUTSIDE_TEMP + String(hive_data.outside_temperature_c)+",";
    //cmd = "AT+SEND=501,"+String(payload.length()) +","+ payload;
    //lora_send_cmd(cmd, NULL, 1);
    //delay(500);

    payload += HIVE_OUTSIDE_HUMID + String(hive_data.outside_humidity)+",";
    //cmd = "AT+SEND=501,"+String(payload.length()) +","+ payload;
    //lora_send_cmd(cmd, NULL, 1);
    //delay(500);

    payload += HIVE_SOUND_LEVEL + String(hive_data.sound_db)+",";
   // cmd = "AT+SEND=501,"+String(payload.length()) +","+ payload;
    //lora_send_cmd(cmd, NULL, 1);
    //delay(500);

    payload += HIVE_BAT_LEVEL + String(hive_data.bat_charge)+",";
    //cmd = "AT+SEND=501,"+String(payload.length()) +","+ payload;
    //lora_send_cmd(cmd, NULL, 1);
    //delay(500);

    payload += HIVE_BAT_STATUS + String(hive_data.bat_status)+",";
    //cmd = "AT+SEND=501,"+String(payload.length()) +","+ payload;
   // lora_send_cmd(cmd, NULL, 1);
    //delay(500);

    payload += HIVE_FAN_STATUS + String(hive_data.fan_status)+",";

    payload += HIVE_CMD_SETTEMP + String(hive_data.set_temp_c)+",";
    
    payload += HIVE_WEIGHT + String(hive_data.raw_weight)+",";

    

    
    String cmd = "AT+SEND=501,"+String(payload.length()) +","+ payload; 
    lora_send_cmd(cmd, NULL, 1);
    //delay(500);
}

void print_data_info(){

  Serial.println(F("------------------------------------"));
  
  Serial.println(F("Sensor and Device Statuses"));
  Serial.print  (F("Temperature: ")); Serial.print(hive_data.inside_temperature_c);Serial.println(F(" C"));
  Serial.print  (F("Humidity: ")); Serial.print(hive_data.inside_humidity);Serial.println(F(" %"));
  Serial.print  (F("Set temp: ")); Serial.print(hive_data.set_temp_c);Serial.println(F(" C"));
  Serial.print  (F("Fan status: ")); Serial.println(hive_data.fan_status==0?"Fan off":"Fan active");
  Serial.print  (F("Scale value: ")); Serial.print(hive_data.raw_weight);Serial.println(F(" lbs"));
  Serial.print  (F("Battery charge: ")); Serial.print(hive_data.bat_charge);Serial.println(F(" %"));
  Serial.print  (F("Battery Status: ")); Serial.println(hive_data.bat_status==0?"Not charging":"Charging");
  Serial.print  (F("Sound level: ")); Serial.print(hive_data.sound_db);Serial.println(F(" dB"));
  
  Serial.println(F("------------------------------------"));

  
}

void scale_calibrate(){
  
  scale.set_scale(-7000.f);
  scale.tare();

  while(1){
      Serial.print("read average: \t\t");
      Serial.println(scale.get_units());       // print the average of 20 readings from the ADC
      //scale.tare();
      delay(250);
  }

  
}

void scale_init(){
  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)

  scale.set_scale(-2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  delay(5000);
  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));    // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  //scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  //scale.set_scale(-7000.f);
  //scale.tare();
  lora_configure();
  dht.begin();
  pinMode(PELTIER_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
     //Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  hive_data.inside_temperature_c = event.temperature;
  hive_data.outside_temperature_c = event.temperature;
  
  dht.humidity().getEvent(&event);
  hive_data.inside_humidity = event.relative_humidity;
  hive_data.outside_humidity = event.relative_humidity;
  
  hive_data.sound_db = analogRead(SOUND_PIN);
  hive_data.bat_charge = 90;
  hive_data.bat_status = 0;
  //hive_data.raw_weight = scale.get_units(5);

  String rx = lora_parse_rx(lora_recieve());
  String cmd = rx.substring(0, HEADER_LENGTH);
  String payload = rx.substring(HEADER_LENGTH);

  if(cmd.equals(HIVE_CMD_FETCH)){
    hive_push_data();
  }
  else if (cmd.equals(HIVE_CMD_TARE)){
    scale.tare();
    hive_data.raw_weight = scale.get_units(5);
    payload = HIVE_WEIGHT + String(hive_data.raw_weight)+",";
    String cmd = "AT+SEND=501,"+String(payload.length()) +","+ payload; 
    lora_send_cmd(cmd, NULL, 1);
  }
  else if (cmd.equals(HIVE_CMD_SETTEMP)){
    hive_data.set_temp_c = payload.toInt();
  }

  if(hive_data.inside_temperature_c - hive_data.set_temp_c > TEMP_HYSTERYSIS){
    //turn on peltier
    hive_data.fan_status = 1;
    digitalWrite(PELTIER_PIN, HIGH);   
  }
  else{
    //turn off peltier
    hive_data.fan_status = 0;
    digitalWrite(PELTIER_PIN, LOW); 
  }
  print_data_info();

  
  //scale.power_down();              // put the ADC in sleep mode
  delay(1000);
  //scale.power_up();
}
