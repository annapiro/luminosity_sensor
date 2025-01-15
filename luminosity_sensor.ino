/* TSL2591 Digital Light Sensor */
/* Dynamic Range: 600M:1 */
/* Maximum Lux: 88K */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// display basic sensor info
void displaySensorDetails(void) {
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  delay(500);
}

// configure gain and integration time
void configureSensor(void) {
  tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  // tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  // tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  // display gain and integration time for reference
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print  (F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC); 
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}

float getLux(void) {
  // Simple data read example. Just read the infrared, fullspecrtrum diode 
  // or 'visible' (difference between the two) channels.
  // This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
  // uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
  // uint16_t x = tsl.getLuminosity(TSL2591_FULLSPECTRUM);
  // uint16_t x = tsl.getLuminosity(TSL2591_INFRARED);

  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  float lux;
  lux = tsl.calculateLux(full, ir);
  Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
  Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
  Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
  Serial.print(F("Visible: ")); Serial.print(full - ir); Serial.print(F("  "));
  Serial.print(F("Lux: ")); Serial.println(tsl.calculateLux(full, ir), 6);

  return lux;
}

uint16_t getChannel0(void) {
  uint16_t full;
  // simple read
  full = tsl.getLuminosity(TSL2591_FULLSPECTRUM);

  // advanced read
  // uint32_t lum = tsl.getFullLuminosity();
  // full = lum & 0xFFFF;
  
  // Serial.print(F("CH0 raw counts: ")); Serial.println(full);

  return full;
}

uint16_t getChannel1(void) {
  uint16_t ir;
  // simple read
  ir = tsl.getLuminosity(TSL2591_INFRARED);

  // advanced read
  // uint32_t lum = tsl.getFullLuminosity();
  // ir = lum >> 16;

  // Serial.print(F("CH1 raw counts: ")); Serial.println(ir);

  return ir;
}

// read using the Adafruit Unified Sensor API
void unifiedSensorAPIRead(void) {
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  /* Display the results (light is measured in lux) */
  Serial.print(F("[ ")); Serial.print(event.timestamp); Serial.print(F(" ms ] "));
  if ((event.light == 0) |
      (event.light > 4294966000.0) | 
      (event.light <-4294966000.0))
  {
    /* If event.light = 0 lux the sensor is probably saturated */
    /* and no reliable data could be generated! */
    /* if event.light is +/- 4294967040 there was a float over/underflow */
    Serial.println(F("Invalid data (adjust gain or timing)"));
  }
  else
  {
    Serial.print(event.light); Serial.println(F(" lux"));
  }
}

void updateDisplay(uint16_t ch0, uint16_t ch1, float lux) {
  // clear the display
  lcd.setCursor(2, 0);
  lcd.print("     ");
  lcd.setCursor(10, 0);
  lcd.print("     ");

  lcd.setCursor(2, 0);
  lcd.print(ch0);
  lcd.setCursor(10, 0);
  lcd.print(ch1);
  lcd.setCursor(6, 1);
  lcd.print(lux);
}

void setup(void) {
  Serial.begin(9600); 
  Serial.println(F("Starting Adafruit TSL2591 test..."));
  
  if (tsl.begin()) 
  {
    Serial.println(F("Found a TSL2591 sensor"));
  } 
  else 
  {
    Serial.println(F("No sensor found!"));
    while (1);
  }
    
  displaySensorDetails();
  configureSensor();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("0:");  // full
  lcd.setCursor(8, 0);
  lcd.print("1:");  // ir
  lcd.setCursor(0, 1);
  lcd.print("Lux:");  // 
}

void loop(void) { 
  float lux = getLux(); 
  uint16_t ch0 = getChannel0();
  uint16_t ch1 = getChannel1();
  updateDisplay(ch0, ch1, lux);
  delay(500);
}
