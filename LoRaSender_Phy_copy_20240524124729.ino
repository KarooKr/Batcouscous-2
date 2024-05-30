#include <SPI.h>
#include <LoRa.h>
#include <FastLED.h> // http://librarymanager/All#FASTLED
#include <LTR303.h>
#include <Wire.h>

int counter = 0;


// Parameters you can play with :


int txPower = 14; // from 0 to 20, default is 14
int spreadingFactor = 12; // from 7 to 12, default is 12
long signalBandwidth = 125E3; // 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3,41.7E3,62.5E3,125E3,250E3,500e3, default is 125E3
int codingRateDenominator=5; // Numerator is 4, and denominator from 5 to 8, default is 5
int preambleLength=8; // from 2 to 20, default is 8
int payload = 0; // you can change the payload

#define SS 10
#define RST 8
#define DI0 3
#define BAND 865E6  // Here you define the frequency carrier
#define LED_PIN     4
#define NUM_LEDS    21
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

#define UPDATES_PER_SECOND 100

LTR303 light;

unsigned char gain;     // Gain setting, values = 0-7 
unsigned char integrationTime;  // Integration ("shutter") time in milliseconds
unsigned char measurementRate;  // Interval between DATA_REGISTERS update






void setup() {
    delay( 3000 ); // power-up safety delay
  // Initialize the Serial port:
  
  Serial.begin(115200);
  
  // Initialize the LTR303 library
  // 100ms 	initial startup time required
  delay(100);

  // You can pass nothing to light.begin() for the default I2C address (0x29)
  light.begin();

  // Get factory ID from sensor:
  // (Just for fun, you don't need to do this to operate the sensor)

  unsigned char ID;
  
  if (light.getPartID(ID)) {
   
  }
  // Most library commands will return true if communications was successful,
  // and false if there was a problem. You can ignore this returned value,
  // or check whether a command worked correctly and retrieve an error code:
  else {
    byte error = light.getError();
    printError(error);
  }

  // The light sensor has a default integration time of 100ms,
  // and a default gain of low (1X).
  
  // If you would like to change either of these, you can
  // do so using the setControl() and setMeasurementRate() command.
  
  // Gain can take any value from 0-7, except 4 & 5
  // If gain = 0, device is set to 1X gain (default)
  // If gain = 1, device is set to 2X gain
  // If gain = 2, device is set to 4X gain
  // If gain = 3, device is set to 8X gain
  // If gain = 4, invalid
  // If gain = 5, invalid
  // If gain = 6, device is set to 48X gain
  // If gain = 7, device is set to 96X gain
  gain = 0;
 
  light.setControl(gain, false, false);

  // If integrationTime = 0, integrationTime will be 100ms (default)
  // If integrationTime = 1, integrationTime will be 50ms
  // If integrationTime = 2, integrationTime will be 200ms
  // If integrationTime = 3, integrationTime will be 400ms
  // If integrationTime = 4, integrationTime will be 150ms
  // If integrationTime = 5, integrationTime will be 250ms
  // If integrationTime = 6, integrationTime will be 300ms
  // If integrationTime = 7, integrationTime will be 350ms

  unsigned char time = 1;

  // If integrationTime = 0, integrationTime will be 100ms (default)
  // If integrationTime = 1, integrationTime will be 50ms
  // If integrationTime = 2, integrationTime will be 200ms
  // If integrationTime = 3, integrationTime will be 400ms
  // If integrationTime = 4, integrationTime will be 150ms
  // If integrationTime = 5, integrationTime will be 250ms
  // If integrationTime = 6, integrationTime will be 300ms
  // If integrationTime = 7, integrationTime will be 350ms
  
 
  light.setMeasurementRate(time,3);

  // To start taking measurements, power up the sensor:
  
 
  light.setPowerUp();
  
  // The sensor will now gather light during the integration time.
  // After the specified time, you can retrieve the result from the sensor.
  // Once a measurement occurs, another integration period will start.

// Setup LED

 FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
 FastLED.setBrightness(  BRIGHTNESS );
    
 currentPalette = RainbowColors_p;
 currentBlending = LINEARBLEND;

  Serial.println("Data0:,Data1:,Lux:");   // Plot labels

  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender");
  Serial.print("SetFrequency : ");
  Serial.print(BAND);
  Serial.println("Hz");
  Serial.print("SetSpreadingFactor : SF");
  Serial.println(spreadingFactor);

  SPI.begin();
  LoRa.setPins(SS,RST,DI0);

  

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
 LoRa.setTxPower(txPower,1);
 LoRa.setSpreadingFactor(spreadingFactor);
 LoRa.setSignalBandwidth(signalBandwidth);
 LoRa.setCodingRate4(codingRateDenominator);
 LoRa.setPreambleLength(preambleLength);
// LoRa.setPolarity(1);
 //LoRa.setFSK(); 
 
}

void loop() {
    // Wait between measurements before retrieving the result
  // You can also configure the sensor to issue an interrupt 
  // when measurements are complete)
  
  // This sketch uses the LTR303's built-in integration timer.
  
  int ms = 50;
  
  delay(ms);
  
  
  // Once integration is complete, we'll retrieve the data.
  
  // There are two light sensors on the device, one for visible light
  // and one for infrared. Both sensors are needed for lux calculations.
  
  // Retrieve the data from the device:

  unsigned int data0, data1;
  double lux;    // Resulting lux value
  boolean good;  // True if neither sensor is saturated

  
  
  if (light.getData(data0,data1)) {
    // getData() returned true, communication was successful
    
    Serial.print(data1);
    Serial.print(" ");
    Serial.print(data0);
    Serial.print(" ");
  
    // To calculate lux, pass all your settings and readings
    // to the getLux() function.
    
    // The getLux() function will return 1 if the calculation
    // was successful, or 0 if one or both of the sensors was
    // saturated (too much light). If this happens, you can
    // reduce the integration time and/or gain.
  
  }  
    
    // Perform lux calculation:

    good = light.getLux(gain,integrationTime,data0,data1,lux);
    
    // Print out the results:
	
  //  Serial.print(" lux: ");
    Serial.println(lux);
//    if (good) Serial.println(" (good)"); else Serial.println(" (BAD)");
//  }
//  else {
//    // getData() returned false because of an I2C error, inform the user.
//
//    byte error = light.getError();
//    printError(error);
//  }
 
uint8_t lux_temp = map(lux, 0,300,170,0); // Map value from luminosity sensor to LED from 170 to 0
//Serial.println(lux_temp);
 // FastLED's built-in rainbow generator
  fill_solid( leds, NUM_LEDS, ColorFromPalette(RainbowColors_p,lux_temp,BRIGHTNESS, LINEARBLEND));

 payload = data0;

  
  
// send the 'leds' array out to the actual LED strip
  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);

  
  // send packet
  
  LoRa.beginPacket();
  LoRa.write(payload);  
  
  LoRa.endPacket();
  counter++;

  //Serial.print("Sending packet with payload {");
  Serial.print("{");
  Serial.print(payload);
  Serial.print("}");
  //Serial.print("} N°");
  //Serial.println(counter);
 
 
  delay(100);
}





void printError(byte error) {
  // If there's an I2C error, this function will
  // print out an explanation.

  Serial.print("I2C error: ");
  Serial.print(error,DEC);
  Serial.print(", ");
  
  switch(error) {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("received NACK on address (disconnected?)");
      break;
    case 3:
      Serial.println("received NACK on data");
      break;
    case 4:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }
}

