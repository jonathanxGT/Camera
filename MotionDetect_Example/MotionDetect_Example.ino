#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"


// comment out this line if using Arduino V23 or earlier
#include <SoftwareSerial.h>
#define chipSelect 10

SoftwareSerial cameraconnection = SoftwareSerial(3, 2);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

RTC_DS1307 RTC;


void setup() {

#if !defined(SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if (chipSelect != 53) pinMode(53, OUTPUT); // SS on Mega
#elseb
  if (chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.
#endif
#endif

  Serial.begin(9600);
  connectToRTC();

  Serial.println("VC0706 Camera test");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }

  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120
  // Remember that bigger pictures take longer to transmit!

  cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");


  //  Motion detection system can alert you when the camera 'sees' motion!
  cam.setMotionDetect(true);           // turn it on
  //cam.setMotionDetect(false);        // turn it off   (default)

  // You can also verify whether motion detection is active!
  Serial.print("Motion detection is ");
  if (cam.getMotionDetect())
    Serial.println("ON");
  else
    Serial.println("OFF");
}




void loop() {

  if (cam.motionDetected()) {
    Serial.println("Motion!");
    cam.setMotionDetect(false);
    Serial.println(cam.motionDetected());
    delay(250);

    if (! cam.takePicture())
      Serial.println("Failed to snap!");
    else
      Serial.println("Picture taken!");
    takePic();
    cam.resumeVideo();
    cam.setMotionDetect(true);
  }
}

void takePic() {
  DateTime fileDate;
  fileDate = RTC.now();

  String fileDatename;
  fileDatename = String(fileDate.month())  + String(fileDate.day())
                 + String(fileDate.hour())  + String(fileDate.minute())
                 + String(fileDate.second()) + '.' + 'J' + 'P' + 'G';
  
    char filename[fileDatename.length()];
    for (uint8_t i = 0; i <= fileDatename.length(); i++) {
      filename[i] = fileDatename.charAt(i);
      if (i == fileDatename.length()) {
        //        if (! SD.exists(filename)) {
        //          break;  // leave the loop!
        //        }
        break;
      }
    }
  
/*
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i / 10;
    filename[6] = '0' + i % 10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  */

  File imgFile = SD.open(filename, FILE_WRITE);

  uint16_t jpglen = cam.frameLength();
  Serial.print(jpglen, DEC);
  Serial.println(" byte image");

  Serial.print("Writing image to "); Serial.print(filename);

  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);

    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");

    jpglen -= bytesToRead;
  }
  imgFile.close();
  Serial.println("...Done!");
}

void createFile() {

}

void connectToRTC() {
  // connect to RTC
  Wire.begin();
  if (!RTC.begin()) {
    Serial.println(F("RTC failed"));

  }
}


