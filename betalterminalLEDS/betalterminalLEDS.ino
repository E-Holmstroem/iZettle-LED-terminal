#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

// Define pins for I2C
#define I2C_SDA 4  // GP4 (Pin 6)
#define I2C_SCL 5  // GP5 (Pin 7)

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// Row 1, for reference GPIO 7 is dead
const int GPIO1x1 = 1; 
const int GPIO1x2 = 6; 
const int GPIO1x3 = 13; 

// Row 2
const int GPIO2x1 = 2; 
const int GPIO2x2 = 8; 
const int GPIO2x3 = 12; 

// Row 3
const int GPIO3x1 = 22; 
const int GPIO3x2 = 20;
const int GPIO3x3 = 17;

// Row 4
const int noGPIO = 16; 
const int GPIO4x2 = 19;
const int yesGPIO = 21; 

//Output
const int LEDdataGPIO = 15;


int btnGPIOs[] = {GPIO1x1, GPIO1x2, GPIO1x3, GPIO2x1, GPIO2x2, GPIO2x3, GPIO3x1, GPIO3x2, GPIO3x3, noGPIO, GPIO4x2, yesGPIO};
const int lenOfBtnGPIOs = 12;

int idle = 1;

struct anyBtn {
  bool val;
  int GPIO;
};

struct anyBtn currBtn;
unsigned long latest_btn_update;
unsigned long latest_Scroll_Update;

char code[] = {' ',' ',' ',' '};
char activeCode[] = {'6','7','6','9'}; // LED pattern active on start
int currCodePos = 0;

char disp[] = {' ',' ',' ',' ','C','A','S','E'};
const int lenOfDisp = 8;
int iterCaseScroll = 0;


char GPIOToInput(int GPIO) {
  Serial.println(GPIO);
  char input; 
  switch (GPIO) {
      case (GPIO1x3):
        input = '1';
        break;
      case (GPIO1x2):
        input = '2';
        break;
      case (GPIO1x1):
        input = '3';
        break;
      case (GPIO2x3):
        input = '4';
        break;
      case (GPIO2x2):
        input = '5';
        break;
      case (GPIO2x1):
        input = '6';
        break;
      case (GPIO3x3):
        input = '7';
        break;
      case (GPIO3x2):
        input = '8';
        break;
      case (GPIO3x1):
        input = '9';
        break;
      case (noGPIO):
        input = 'n';
        break;
      case (GPIO4x2):
        input = '0';
        break;
      case (yesGPIO):
        input = 'y';
        break;

      default:
        input = '?'; 
        break;
  }
  return input;
}

void resetCode() {
  code[0] = ' ';
  code[1] = ' ';
  code[2] = ' ';
  code[3] = ' ';
  currCodePos = 0;
}

void reset() {
  resetCode();  
  idle=1;
}

void updateLEDs() {
  static char lastCode[4] = {0, 0, 0, 0};

  if (memcmp(activeCode, lastCode, 4) == 0) return;
  memcpy(lastCode, activeCode, 4);

  // Serial.print("Active code: ");
  Serial.write((uint8_t*)activeCode, 4);
  // Serial.println();

  if (memcmp(activeCode, "1231", 4) == 0) {
    // simple test, will be switched out for LED logic later
    alpha4.writeDigitAscii(0, '?');
    alpha4.writeDigitAscii(1, '!');
    alpha4.writeDigitAscii(2, '?');
    alpha4.writeDigitAscii(3, '?');
    alpha4.writeDisplay();

    delay(2500);
  }
}

struct anyBtn anyBtnActive() {
  struct anyBtn btn;
  for (int i = 0; i < lenOfBtnGPIOs; i++) {
    bool GPIOValue = digitalRead(btnGPIOs[i]);
    if (!GPIOValue) { //Only first pin will be noticed, which is fine for such a simple interface.
      btn.val = true;
      btn.GPIO = btnGPIOs[i];
      return btn;
    }
  }
  btn.val = false;
  btn.GPIO = 0;
  return btn;
}

void caseScroll() {
  if (iterCaseScroll==lenOfDisp) {iterCaseScroll=0;}
  alpha4.writeDigitAscii(0, disp[ (abs(lenOfDisp - iterCaseScroll)+0) % lenOfDisp] );
  alpha4.writeDigitAscii(1, disp[ (abs(lenOfDisp - iterCaseScroll)+1) % lenOfDisp] );
  alpha4.writeDigitAscii(2, disp[ (abs(lenOfDisp - iterCaseScroll)+2) % lenOfDisp] );
  alpha4.writeDigitAscii(3, disp[ (abs(lenOfDisp - iterCaseScroll)+3) % lenOfDisp] );

  alpha4.writeDisplay();

  latest_Scroll_Update = millis();
  iterCaseScroll++;
}

void display() {
  alpha4.writeDigitAscii(0, code[0]);
  alpha4.writeDigitAscii(1, code[1]);
  alpha4.writeDigitAscii(2, code[2]);
  alpha4.writeDigitAscii(3, code[3]);

  alpha4.writeDisplay();
}

//--------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);

  // Assign the I2C pins specifically for the Pico architecture
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);

  // set all GPIO to input_pullup
  for(int pin = 0 ; pin < lenOfBtnGPIOs ; pin++) {
    pinMode(btnGPIOs[pin], INPUT_PULLUP);
  }

  // Start the I2C bus
  Wire.begin(); 

  // Initialize the HT16K33 backpack (default address 0x70)
  if (!alpha4.begin(0x70)) {
    while (1){
      Serial.println("Hey this shit aint working");

      // Might add status LED
      // Perhaps check buttonpress to retry setup()
    };
  }
  
  alpha4.setBrightness(1); // (0 to 15)

  alpha4.writeDigitAscii(0, 'H');
  alpha4.writeDigitAscii(1, 'e');
  alpha4.writeDigitAscii(2, 'l');
  alpha4.writeDigitAscii(3, 'o');

  alpha4.writeDisplay();

  delay(2500);
}

//--------------------------------------------------------------------------------------------------

void loop() {
  // check buttons
  unsigned long btnDeltat = millis()-latest_btn_update;
  if (btnDeltat > 200) {
    Serial.println(digitalRead(7));
    currBtn = anyBtnActive();
    if (currBtn.val == true) {
      latest_btn_update = millis();
      idle = 0;

      // if you try to write more than 4 numbers code will reset.
      if (currCodePos == 4 && ( currBtn.GPIO != yesGPIO && currBtn.GPIO != noGPIO )) {resetCode();}
      else if (currBtn.GPIO == yesGPIO) { // Send the code currently displayed
        activeCode[0] = code[0];
        activeCode[1] = code[1];
        activeCode[2] = code[2];
        activeCode[3] = code[3];

        updateLEDs();
        iterCaseScroll=0;
        reset();
      }
      else if (currBtn.GPIO == noGPIO) { // Go back to idle screen
        iterCaseScroll=0;
        reset();
      }
      else {
        // save char to current code position
        code[currCodePos] = GPIOToInput(currBtn.GPIO);
        currCodePos++;  
      }
      
    }
  } 
  if (btnDeltat > 7500) { // if you don't do anything for 7.5 sec on the code scrren go back to idle
    latest_btn_update = millis();
    reset();
  }
  
  // Update the pattern for leds
  // updateLEDs();


  // Render on display according to idle or not
  if (!idle) {
    display();
  } 
  else if ((millis()-latest_Scroll_Update) > 275) {
    caseScroll();
  }
}
