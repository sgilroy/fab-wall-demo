#include "FastLED.h"
#include "Sensor.h"
#include "ToggleButton.h"
#include "PirSensor.h"
#include <Playtune.h>

const byte PROGMEM motionEndTune [] = {
  //  7,208, 0x90,0x45, 0x91,0x39, 1,77, 0x80, 0x81, 0x90,0x44, 0,166, 0x80, 0x90,0x45, 0,166, 0x80, 0x90,0x47,
  //  0xf0
  0x90, 88, 0, 75, 0x80, 0x90, 83, 0, 225, 0x80, 0xf0
};

const byte PROGMEM motionStartTune [] = {
  //  0x91,0x38, 1,77, 0x80, 0x81, 0x90,0x45, 0,166, 0x80, 0x90,0x44, 0,166, 0x80, 0x90,0x45, 0x91,0x36, 1,77,
  //  0xf0
  0x90, 83, 0, 75, 0x80, 0x90, 88, 0, 225, 0x80, 0xf0
};

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    7
#define CLK_PIN   4
#define LED_TYPE    DOTSTAR
#define COLOR_ORDER GBR
#define NUM_LEDS    240
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120
unsigned int button1Pin = 2;

Sensor * s1;
ToggleButton * toggleButton1;

//class PirSensor2 {};
PirSensor * pirSensor;
Playtune pt;

void setup() {
  delay(3000); // 3 second delay for recovery

  pt.tune_initchan (5);
  s1 = new Sensor(12, 9);
  pirSensor = new PirSensor(3);
  toggleButton1 = new ToggleButton(2);
  Serial.begin(115200);

  // tell FastLED about the LED strip configuration
  //  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  resetBouncePosition();
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
//SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
//SimplePatternList gPatterns = { confetti, barSlide, bounce };
//SimplePatternList gPatterns = { barSlide };
//SimplePatternList gPatterns = { barSlide, doFireflies, bounce };
SimplePatternList gPatterns = { bounce, barSlide, doFireflies, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
//SimplePatternList gPatterns = { graph };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

unsigned long dist1 = 0;
bool showGraph = false;
enum PirSensor::motionTransitions motionTransition;
bool sleeping = true;

void loop()
{
  showGraph = toggleButton1->update();

  updateSleeping();

  if (!sleeping) {
    if (showGraph) {
      dist1 = s1->dist();
      graph();
    } else {
      // Call the current pattern function once, updating the 'leds' array
      gPatterns[gCurrentPatternNumber]();
    }
  } else {
    standbyBreath();
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 10 ) {
    if (random(1, 100) > 50) {
      gHue++;
    }
  } // slowly cycle the "base color" through the rainbow

  EVERY_N_SECONDS( 10 ) {
    nextPattern();  // change patterns periodically
  }

#ifdef DEBUG_DIST1
  Serial.print(dist1);
  Serial.print("\n");
#endif
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
uint8_t born = 0;

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  born = 0;
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void graph()
{
  for (CRGB & pixel : leds) {
    pixel = CRGB::Black;
  }
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, dist1, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

#define NUM_FIREFLIES NUM_LEDS / 5
typedef struct {
  uint8_t position;
  long age;
  uint8_t hue;
} Firefly;
Firefly fireflies[NUM_FIREFLIES];

void createFirefly(int i) {
  fireflies[i].position = random16(NUM_LEDS);
  //    fireflies[i].position = i;
  fireflies[i].age = 0;
  fireflies[i].hue = gHue + random8(64);
}

void doFireflies()
{
  // random colored speckles that fade in and out smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  //  leds[pos] += CHSV( gHue + random8(64), 200, 255);

  if (random(0, 100) > 80) {
    if (born < NUM_FIREFLIES) {
      createFirefly(born);
      born++;
    }
  }
  for (int i = 0; i < born; i++) {
    fireflies[i].age += 3;
    if (fireflies[i].age <= 255) {
      leds[fireflies[i].position] = CHSV( fireflies[i].hue, 200, fireflies[i].age);
    } else if (fireflies[i].age < 512) {
      leds[fireflies[i].position] = CHSV( fireflies[i].hue, 200, 511 - fireflies[i].age);
      //        leds[fireflies[i].position] = CHSV( fireflies[i].hue, 200, 128);
    } else {
      // reborn
      leds[fireflies[i].position] = CHSV( fireflies[i].hue, 200, 0);
      createFirefly(i);
    }
  }
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


// Draw a "Fractional Bar" of light starting at position 'pos16', which is counted in
// sixteenths of a pixel from the start of the strip.  Fractional positions are
// rendered using 'anti-aliasing' of pixel brightness.
// The bar width is specified in whole pixels.
// Arguably, this is the interesting code.
void drawFractionalBar( int pos16, int width, uint8_t hue)
{
  pos16 = constrain(pos16, 0, NUM_LEDS * 16);
  int i = pos16 / 16; // convert from pos to raw pixel number
  uint8_t frac = pos16 & 0x0F; // extract the 'factional' part of the position

  // brightness of the first pixel in the bar is 1.0 - (fractional part of position)
  // e.g., if the light bar starts drawing at pixel "57.9", then
  // pixel #57 should only be lit at 10% brightness, because only 1/10th of it
  // is "in" the light bar:
  //
  //                       57.9 . . . . . . . . . . . . . . . . . 61.9
  //                        v                                      v
  //  ---+---56----+---57----+---58----+---59----+---60----+---61----+---62---->
  //     |         |        X|XXXXXXXXX|XXXXXXXXX|XXXXXXXXX|XXXXXXXX |
  //  ---+---------+---------+---------+---------+---------+---------+--------->
  //                   10%       100%      100%      100%      90%
  //
  // the fraction we get is in 16ths and needs to be converted to 256ths,
  // so we multiply by 16.  We subtract from 255 because we want a high
  // fraction (e.g. 0.9) to turn into a low brightness (e.g. 0.1)
  uint8_t firstpixelbrightness = 255 - (frac * 16);

  // if the bar is of integer length, the last pixel's brightness is the
  // reverse of the first pixel's; see illustration above.
  uint8_t lastpixelbrightness  = 255 - firstpixelbrightness;

  // For a bar of width "N", the code has to consider "N+1" pixel positions,
  // which is why the "<= width" below instead of "< width".

  uint8_t bright;
  for ( int n = 0; n <= width; n++) {
    if ( n == 0) {
      // first pixel in the bar
      bright = firstpixelbrightness;
    } else if ( n == width ) {
      // last pixel in the bar
      bright = lastpixelbrightness;
    } else {
      // middle pixels
      bright = 255;
    }

    leds[i] += CHSV( hue, 255, bright);
    i++;
    if ( i == NUM_LEDS) i = 0; // wrap around
  }
}


int position = 0; // position of the "fraction-based bar"
int velocity = 0;
int F16delta = 30; // how many 16ths of a pixel to move the Fractional Bar
int Width  = 4; // width of each light bar, in whole pixels

void clear() {
  // clear the pixel buffer
  //   memset8(leds, 0, NUM_LEDS * sizeof(CRGB));
  fadeToBlackBy( leds, NUM_LEDS, 20);
}

void barSlide() {
  // Update the "Fraction Bar" by 1/16th pixel every time
  velocity = F16delta;
  position += velocity;

  // wrap around at end
  // remember that position contains position in "16ths of a pixel"
  // so the 'end of the strip' is (NUM_LEDS * 16)
  if ( position >= (NUM_LEDS * 16)) {
    position -= (NUM_LEDS * 16);
  }


  // Draw everything:
  clear();

  // draw the Fractional Bar, length=4px
  drawFractionalBar(position, Width, gHue);
}

void drip()
{
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bounce()
{
  clear();
  drawFractionalBar(position, Width, gHue);

  const int gravityCentimetersPerSecondSquared = -980;
  const int pixelsPerMeter = 60;
  //  const int halfDegreesPerLoop = 720;
  const int halfDegreesPerLoop = 16 * NUM_LEDS;
  const int acceleration = (((long) gravityCentimetersPerSecondSquared * pixelsPerMeter / 100) * halfDegreesPerLoop / NUM_LEDS) / FRAMES_PER_SECOND;
  velocity += acceleration;
  position += velocity / FRAMES_PER_SECOND;

  if (position <= 0) {
    // bounce back with slightly less velocity
    int newVelocity = (((long) velocity) * -90) / 100;
    if (newVelocity <= -acceleration)
    {
      resetBouncePosition();
    }
    else
    {
      velocity = newVelocity;
      position = 0;
    }
  }
}

void resetBouncePosition()
{
  velocity = 0;
  position = (NUM_LEDS - Width) * 16; // position (start near the top)
}

const int buzzerPin = 5;

#define  c     3830    // 261 Hz 
#define  d     3400    // 294 Hz 
#define  e     3038    // 329 Hz 
#define  f     2864    // 349 Hz 
#define  g     2550    // 392 Hz 
#define  a     2272    // 440 Hz 
#define  b     2028    // 493 Hz 
#define  C     1912    // 523 Hz 

unsigned long motionEndTime = 0;
const unsigned long SLEEP_DELAY = 1000.0 * 60 * 5;

void updateSleeping() {
  motionTransition = pirSensor->update();

  if (motionTransition == PirSensor::END) {
    motionEndTime = millis() + SLEEP_DELAY;
  } else if (motionTransition == PirSensor::START && sleeping) {
    pt.tune_playscore (motionStartTune);
  }

  if (pirSensor->pirState == LOW && millis() > motionEndTime) {
    if (!sleeping) {
      //    tone(buzzerPin, d, 20);
      //    tone(buzzerPin, C, 20);
      pt.tune_playscore (motionEndTune);
    }
    sleeping = true;
  } else {
    sleeping = false;
  }
}
void standbyBreath() {
  fadeToBlackBy(leds, NUM_LEDS, 20);

  float breath = (exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0;
  leds[0] = CHSV(gHue, 255, breath * 0.25 + 64);
}

