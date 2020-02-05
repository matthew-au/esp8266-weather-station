/* ESP8266 Nodemcu with BMP180, DHT11 and SSD1306 128x64 OLED
 *  Matthew-au 2019
 *  code snippets from circuits4you.com and Arduino.cc
 *  Libraries all open source
 
Hardware connections:
NodeMCU     BMP180
3.3V         VIN
GND          GND
D1           SCL          
D2           SDA

DHT11 out to 2 (D4 on nodemcu)

OLED to D1, D2 i2c connections per above

3.3V only for all sensors
 
*/
 
#include <SFE_BMP180.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define DHTPIN 2     // connected to nodemcu Digital pin D4 (see pin mapping sheet)
#define DHTTYPE DHT11   // DHT 11 sensor type

int maxHum = 80; // max reliable humidity of DHT11 sensor
int maxTemp = 40; // max reliable temperature of DHT11 sensor
DHT dht(DHTPIN, DHTTYPE); // create an instance of DHT

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire); //specifying display type with ADAFRUIT library
 
SFE_BMP180 pressure; // creates a SFE_BMP180 object called "pressure":
 
#define ALTITUDE 50.0 // Altitude in meters ADELAIDE SOUTH AUSTRALIA


 
void setup()
{
  Serial.begin(9600); //start serial
  dht.begin(); //start DHT
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally with address 0x3C for ebay 128x64
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  //oled init switch on delay
  delay(1000);
 
  // Clear the oled display buffer.
  display.clearDisplay();


  // Initialize the sensor (it is important to get calibration values stored on the device).
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}
 
void loop()
{
  char status;
  double T,P,p0,a; // variables for measurements

// header line of oled display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Matts Weather Station");


   // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:
  
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE*3.28084,0);
  Serial.println(" feet");
  
  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.
 
  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.
 
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
 
    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.
 
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");

// send temp variable to oled
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,12);
  display.print("Temp: ");
  display.print(T,2);
  display.println(" deg C");

      
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.
 
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
 
        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.
 
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");

// send pressure variable to oled
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,24);
  display.print("Pres: ");
  display.print(P,2);
  display.println(" mbars");

 
          // The pressure sensor returns absolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sea level function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb
 
          p0 = pressure.sealevel(P,ALTITUDE); // we're at 50 metres ADELAIDE
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");
 
          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.
 
          a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");

// send altitude variable to oled
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,36);
  display.print("Alt:  ");
  display.print(a,0);
  display.println(" Metres");

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  
  // Check if any readings failed
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Serial.print("DHT11 Readings:");
  Serial.print("\n"); 
  Serial.print("Humidity: "); 
  Serial.print(h);
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,48);
  display.print("Hum:  ");
  display.print(h,0);
  display.println(" %");
  display.display(); // actually display all of the above
  

        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");

  display.display(); // actually display all of the above

 
  delay(5000);  // Pause for 5 seconds.
  display.clearDisplay();
  

}
