// Temperature gauge using the round LCD display from Waveshare with ESP32S3 and a thermocouple sensor from Adafruit

// created by upir, 2024
// youtube channel: https://www.youtube.com/upir_upir

// YouTube video: 
// Source files: 

// Links from the video:
// Do you like this video? You can buy me a coffee ☕: https://www.buymeacoffee.com/upir
// Display with enclosure: https://s.click.aliexpress.com/e/_DkQiwQf
// Display without enclosure: https://s.click.aliexpress.com/e/_DEe0YJv
// Display documentation: https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28
// Adapter board (1.27mm to 2.54mm Pitch): https://s.click.aliexpress.com/e/_Dc74hqb
// Image2cpp (convert array to image): https://javl.github.io/image2cpp/
// Photopea (online graphics editor like Photoshop): https://www.photopea.com/
// Custom pin header adapter: https://www.pcbway.com/project/shareproject/Waveshare_ESP32_S3_LCD_1_28_PCB_for_converting_pins_1_27_2_54mm_d1cfb087.html
// USB-C adapter: https://s.click.aliexpress.com/e/_DD2cHkT
// USB-C multiple cables: https://s.click.aliexpress.com/e/_DE8b0UP
// Screw Terminals: https://s.click.aliexpress.com/e/_DnB938b
// ESP32 Partition calculator: https://esp32.jgarrettcorbin.com/
// Adafruit Universal Thermocouple Amplifier MAX31856: https://www.adafruit.com/product/3263
// Thermocouple Type-K sensor: https://www.adafruit.com/product/270

// Related videos:
// CHEAP DIY BOOST GAUGE: https://youtu.be/cZTx7T9uwA4
// Pitch and roll indicator: https://youtu.be/GosqWcScwC0
// Custom Shifter Knob with Display: https://www.youtube.com/playlist?list=PLjQRaMdk7pBb6r6xglZb92DGyWJTgBVaV



#include <TFT_eSPI.h> // Include the graphics library for drawing on the display
TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

#include <Adafruit_MAX31856.h> // thermocouple sensor breakout board - Adafruit Universal Thermocouple Amplifier MAX31856 Breakout
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(/*CS*/14, /*DI*/15, /*DO*/16, /*CLK*/17); // thermocouple board connection

// all the images are created in Photopea, exported using image2cpp website and stored in separate header files
#include "images.h" // background image stored in a header file
#include "images_digits.h" // 10 digits + empty digit images
#include "images_needle.h" // 121 needle images

// variables for measuring time to take a new temperature measurement every 1 second
unsigned long time_now; // current time
unsigned long time_previous; // previously measured time

float temperature_sensor; // thermocouple sensor temperature in °C
int needle_image; // which needle image to show - stored as 121 separate images
int gauge_min_value = 40; // minimum temperature in °C for the needle images
int gauge_max_value = 120; // maximum temperature in °C for the needle images

int value_temp_digits = 0; // value to be displayed on the display
float temperature_interpolated; // interpolated temperature value between the last measurement and the new measurement to show a smooth transition between temperatures


void setup()
{

  // pins 18 and 21 are used as power for the sensor, since the pin headers do not include 3.3V pin
  // this is not be best solution, but it seems to be working
  pinMode(18, OUTPUT); // set pinmode to output so we can set it HIGH or LOW
  pinMode(21, OUTPUT);  // set pinmode to output so we can set it HIGH or LOW
  digitalWrite(18, HIGH); // 3.3V
  digitalWrite(21, LOW); // GND
  
  Serial.begin(115200); // Use Serial as debugging serial port (sending data back to PC), not used for final sketch

  maxthermo.begin(); // start the thermocouple sensor
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K); // our thermocouple probe is Type K (a very common type)
  maxthermo.setConversionMode(MAX31856_ONESHOT_NOWAIT); // set the mode to ONESHOT_NOWAIT, this requires calling two functions - trigger one shot, wait one second, and take measurement

  tft.init(); // initialize the display
  tft.setRotation(4); // set the display rotation, in this rotation, the USB port is on the bottom
  tft.fillScreen(TFT_DARKGREY); // fill the display with a dark grey color
  tft.setTextFont(4); // set the font, font number 4 is quite big, font was only used for debugging, final numbers are drawn with images
  tft.setSwapBytes(true); // Swap the colour byte order when rendering, requires for the used color format

  tft.pushImage(0, 0, 240, 240, epd_bitmap_allArray[0]); // draw the fullscreen image, only once
}



void loop()
{

  // only take a new measurement every 1 second
  time_now = millis(); // get the time from starting the sketch
  if ((time_now - time_previous) > 1000) { // take a new measurement, if the elapsed time is bigger than 1 second
    temperature_sensor = maxthermo.readThermocoupleTemperature(); // take a new temperature measurement
    time_previous = time_now; // update the previous time

    maxthermo.triggerOneShot(); // trigger a new measurement, we need to wait until we can read the new temperature
  }

  // debug only, show digits from 0-999
  //value_temp_digits++;
  //if (value_temp_digits > 999) {value_temp_digits = 0;}

  // smoothly update the displayed temperature between the last value and a new value
  // increase the number value for a slower update
  temperature_interpolated = temperature_interpolated + ((temperature_sensor - temperature_interpolated) / 5.0);
  value_temp_digits = round(temperature_interpolated); // round the value to the integer value
  value_temp_digits = constrain(value_temp_digits, 0, 999); // constrain the value to only go between 0-999, since this is all we can display with our digits

  // draw digits on the bottom of the screen, centered
  if (value_temp_digits < 10) { // draw one digit
    tft.pushImage(66, 181, 36, 44, bitmaps_digits[10]);
    tft.pushImage(102, 181, 36, 44, bitmaps_digits[value_temp_digits]);    
    tft.pushImage(138, 181, 36, 44, bitmaps_digits[10]);
  } else if (value_temp_digits < 100) { // draw two digits
    tft.pushImage(66, 181, 18, 44, bitmaps_digits[10]);
    tft.pushImage(84, 181, 36, 44, bitmaps_digits[(value_temp_digits % 100) / 10]);    
    tft.pushImage(120, 181, 36, 44, bitmaps_digits[value_temp_digits % 10]);
    tft.pushImage(156, 181, 18, 44, bitmaps_digits[10]);
  } else if (value_temp_digits < 1000) { // draw three digits
    tft.pushImage(66, 181, 36, 44, bitmaps_digits[value_temp_digits / 100]);
    tft.pushImage(102, 181, 36, 44, bitmaps_digits[(value_temp_digits % 100) / 10]);    
    tft.pushImage(138, 181, 36, 44, bitmaps_digits[value_temp_digits % 10]);
  }

  // draw the needle image
  // the map function is not working properly with small values, so we multiply it by 10 and divide by 10 in the next step
  needle_image = map(temperature_interpolated*10.0, gauge_min_value*10.0, gauge_max_value*10.0, 0*10.0, 120*10.0);
  needle_image = round(needle_image/10.0);
  needle_image = constrain(needle_image, 0, 120); // we only have 121 images, restrict the value to 0-120 range

  // the needle image is above all the digits and they are not intersecting, as without using sprites, this would cause flickering
  tft.pushImage(11, 11, 218, 170, bitmaps_needle[needle_image]); // draw the needle image
  

  // debug only - print text on the display
/*
  // print temperature values on the display
  // CJ - cold junction - temperature of the chip
  // TC - thermocouple - temperature of the thermocouple probe
  tft.setCursor(70, 180);
  tft.print("CJ: ");
  tft.print(maxthermo.readCJTemperature());
  tft.print("   ");

  tft.setCursor(70, 205);
  tft.print("TC: ");
  tft.print(maxthermo.readThermocoupleTemperature());
  tft.print("   ");*/

  //delay(1000);

}

