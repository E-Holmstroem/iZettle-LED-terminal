#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <FastLED.h>


// Define pins for I2C
#define I2C_SDA 4  // GP4 (Pin 6)
#define I2C_SCL 5  // GP5 (Pin 7)


// LEDS
int startHue = 0;
int codeInt = 69; // start value for LED animation
#define LED_PIN     15       
#define NUM_LEDS    215  
#define BRIGHTNESS  100 // (0 - 255)
#define LED_TYPE    WS2815
#define COLOR_ORDER RGB

CRGB leds[NUM_LEDS];

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// Row 1, for reference GPIO 7 is dead
const int GPIO1x1 = 1; 
const int GPIO1x2 = 9; 
const int GPIO1x3 = 13; 

// Row 2
const int GPIO2x1 = 18; 
const int GPIO2x2 = 8; 
const int GPIO2x3 = 11; 

// Row 3
const int GPIO3x1 = 22; 
const int GPIO3x2 = 20;
const int GPIO3x3 = 17;

// Row 4
const int noGPIO = 16; 
const int GPIO4x2 = 19;
const int yesGPIO = 21; 


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
char activeCode[] = {' ',' ',' ',' '};
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

void updateLEDs() {

  /* possible future codes
    - 420: Some form of green pattern 'bLAz' 
    - 0000: turn of leds (not the same as "    " or "00  ", specifically "0000" will work) 'Anon'
    - 255: intense white flashing (reference to max brightness value.) 'FLsH'
    - 911 / 112: Red and blue blinking lights 'COP!'
    - 3112: wireworks (date 31/12 new year) 'fWrk'
    - should make it so that i can access menus for changing the brightness or 

  */
  if (memcmp(activeCode, "0000", 4) == 0) { // blackout
    codeInt = 0;
    
    char text[4] = {'A', 'n', 'o', 'n'};
    prepDisplay(text);
  }
  else if (memcmp(activeCode, "67  ", 4) == 0) {
    codeInt = 67;
    // alpha4.writeDigitAscii(0, 'R');
    // alpha4.writeDigitAscii(1, 'e');
    // alpha4.writeDigitAscii(2, 'd', true);
    // alpha4.writeDigitAscii(3, ' ');
    char text[4] = {'h', 'e', 'h', 'e'};
    prepDisplay(text);
  }
  else if (memcmp(activeCode, "69  ", 4) == 0) {
    codeInt = 69;
    // alpha4.writeDigitAscii(0, 'C', true);
    // alpha4.writeDigitAscii(1, 'A', true);
    // alpha4.writeDigitAscii(2, 'S', true);
    // alpha4.writeDigitAscii(3, 'E');
    char text[4] = {'C', 'A', 'S', 'E'};
    bool dots[4] = {true, true, true, false};
    prepDisplay(text, dots);
  }
  else if (memcmp(activeCode, "255 ", 4) == 0) {
    codeInt = 255;
    char text[4] = {'F', 'l', 'S', 'H'};
    prepDisplay(text);
  }
  else if (memcmp(activeCode, "420 ", 4) == 0) {
    codeInt = 420;
    char text[4] = {'b', 'L', 'A', 'z'};
    prepDisplay(text);
  }
  else {
    // alpha4.writeDigitAscii(0, 'N');
    // alpha4.writeDigitAscii(1, 'A');
    // alpha4.writeDigitAscii(2, 'N', true);
    // alpha4.writeDigitAscii(3, ' ');
    char text[4] = {'N', 'A', 'N', ' '};
    bool dots[4] = {false, false, true, false};
    prepDisplay(text, dots);

  }
  
  FastLED.setBrightness(BRIGHTNESS);
  alpha4.writeDisplay();
  delay(1000);
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
  // alpha4.writeDigitAscii(0, disp[ (abs(lenOfDisp - iterCaseScroll)+0) % lenOfDisp] );
  // alpha4.writeDigitAscii(1, disp[ (abs(lenOfDisp - iterCaseScroll)+1) % lenOfDisp] );
  // alpha4.writeDigitAscii(2, disp[ (abs(lenOfDisp - iterCaseScroll)+2) % lenOfDisp] );
  // alpha4.writeDigitAscii(3, disp[ (abs(lenOfDisp - iterCaseScroll)+3) % lenOfDisp] );

  char chars[4] = {disp[ (abs(lenOfDisp - iterCaseScroll)+0) % lenOfDisp], disp[ (abs(lenOfDisp - iterCaseScroll)+1) % lenOfDisp], disp[ (abs(lenOfDisp - iterCaseScroll)+2) % lenOfDisp], disp[ (abs(lenOfDisp - iterCaseScroll)+3) % lenOfDisp]};
  prepDisplay(chars);
  alpha4.writeDisplay();

  latest_Scroll_Update = millis();
  iterCaseScroll++;
}

// Display n such
void prepDisplay(const char chars[4], const bool dot[4]) {
  alpha4.writeDigitAscii(0, chars[0], dot[0]);
  alpha4.writeDigitAscii(1, chars[0], dot[0]);
  alpha4.writeDigitAscii(2, chars[0], dot[0]);
  alpha4.writeDigitAscii(3, chars[0], dot[0]);
}

void prepDisplay(const char chars[4]) {
  const bool noDots[4] = {false, false, false, false};
  prepDisplay(chars, noDots);
}

void display() {
  // alpha4.writeDigitAscii(0, code[0]);
  // alpha4.writeDigitAscii(1, code[1]);
  // alpha4.writeDigitAscii(2, code[2]);
  // alpha4.writeDigitAscii(3, code[3]);

  char chars[4] = {code[0], code[1], code[2], code[3]};
  prepDisplay(chars);
  alpha4.writeDisplay();
}

void displayLEDs() {
  int portion = 8;

  switch (codeInt) {
    case(0): // all LEDs go dark
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.setBrightness(0);
      break;
    
    case (67): // alternating up and down rainbows
      if (startHue < 256/portion){
        fill_rainbow_circular(leds, NUM_LEDS, startHue, true);
        fill_solid(leds, NUM_LEDS*(portion-1)/portion, CRGB::Black);
        fill_rainbow_circular(leds, NUM_LEDS/portion, (startHue+(256/2))%256, true);
      }
      else if (startHue < (256/portion)*2) {
        fill_rainbow_circular(leds, NUM_LEDS, startHue, false);
        fill_solid(leds, NUM_LEDS*(portion-1)/portion, CRGB::Black);
        fill_rainbow_circular(leds, NUM_LEDS/portion, (startHue+(256/2))%256, false);
      } else {startHue = 0;}
      startHue++; 
      
      // fill_solid(leds, NUM_LEDS, CRGB::Green);
      break;

    case(69): //CASE rainbow
      fill_rainbow_circular(leds, NUM_LEDS, startHue, false);
      // fill_rainbow(leds, NUM_LEDS, startHue, 1);
      startHue++; 
      if(startHue==256){startHue=0;}
      break;

    case(255): // aggressive flash
      FastLED.setBrightness(255*min(1, (startHue%3)));
      fill_solid(leds, NUM_LEDS, CRGB::White);
      startHue++;
      if(startHue==3){startHue=0;}
      break;

    case(420): // green

      break;

    case(911): // blinking blue and red

      break;

    default: // ten lonely blue LEDs will indicate an error
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      fill_solid(leds, 10, CRGB::Blue);
      break;
  }
  FastLED.show();
  
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

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  
  alpha4.setBrightness(1); // (0 to 15)

  // alpha4.writeDigitAscii(0, 'H');
  // alpha4.writeDigitAscii(1, 'e');
  // alpha4.writeDigitAscii(2, 'l');
  // alpha4.writeDigitAscii(3, 'o');

  char chars[4] = {'H','e','l','o'};
  prepDisplay(chars);
  alpha4.writeDisplay();

  delay(2500);
}

//--------------------------------------------------------------------------------------------------

void loop() {
  // check buttons
  unsigned long btnDeltat = millis()-latest_btn_update;
  if (btnDeltat > 200) {
    // Serial.println(digitalRead(7));
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
  displayLEDs();


  // Render on display according to idle or not
  if (!idle) {
    display();
  } 
  else if ((millis()-latest_Scroll_Update) > 275) {
    caseScroll();
  }
}
