#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

// Define pins for I2C
#define I2C_SDA 4  // GP4 (Pin 6)
#define I2C_SCL 5  // GP5 (Pin 7)

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

int btnPins[] = {1,1,1,1,1};
int lenOfBtnPins = 5;

struct anyBtn {
  bool btn;
  int pin;
};

void updateLEDs() {
  return;
}

struct anyBtn anyBtn() {
  struct anyBtn btn;
  for (int i = 0; i < lenOfBtnPins; i++) {
    int pinValue = digitalRead(btnPins[i]);
    if (pinValue) {
      btn.btn = true;
      btn.pin = btnPins[i];
      return btn;
    }
  }
  btn.btn = false;
  btn.pin = 0;
  return btn;
}

struct anyBtn caseScroll() {
  char disp[] = {' ',' ',' ',' ','C','A','S','E'};
  int lenOfDisp = 8;
  int i = 0;
  struct anyBtn btn = anyBtn();

  do {
    if (i==lenOfDisp){i=0;}
    alpha4.writeDigitAscii(0, disp[(abs(lenOfDisp-i)+0)%lenOfDisp]);
    alpha4.writeDigitAscii(1, disp[(abs(lenOfDisp-i)+1)%lenOfDisp]);
    alpha4.writeDigitAscii(2, disp[(abs(lenOfDisp-i)+2)%lenOfDisp]);
    alpha4.writeDigitAscii(3, disp[(abs(lenOfDisp-i)+3)%lenOfDisp]);

    alpha4.writeDisplay();

    int t = millis();
    while(((millis()-t) < 275) && !btn.btn ) {
      // updateLEDs();
      btn = anyBtn();
    }

    i++;
  } while (!(btn.btn));
  return btn;
}

void setup() {
  Serial.begin(9600);

  // Assign the I2C pins specifically for the Pico architecture
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  
  // Start the I2C bus
  Wire.begin(); 

  // Initialize the HT16K33 backpack (default address 0x70)
  if (!alpha4.begin(0x70)) {
    Serial.println("HT16K33 backpack not found! Check your wiring.");
    while (1){
      Serial.println("Hey this shit aint working");

      // Might add status LED
      // Check buttonpress to retry setup()
    };
  }
  
  Serial.println("HT16K33 initialized successfully.");
  alpha4.setBrightness(1); // (0 to 15)

  alpha4.writeDigitAscii(0, 'H');
  alpha4.writeDigitAscii(1, 'e');
  alpha4.writeDigitAscii(2, 'l');
  alpha4.writeDigitAscii(3, 'o');

  alpha4.writeDisplay();

  delay(2500);
}

void loop() {
  // gonna try to take btn check out of casescroll

  caseScroll();

  while(true){}


}
