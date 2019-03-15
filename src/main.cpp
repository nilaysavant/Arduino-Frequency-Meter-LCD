/*
* Freq Measure and display to i2c LCD
* Author: Nilay Savant
* Description: ...
* Ref: Using Nick Gammons's Timer/Counter example code
* Input: Pin D5
*/
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // For i2c LCD

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned int c_init = 0;
unsigned int counts = 0;
unsigned int counts_store = 0;
byte index = 0;
bool first = true;

const int timer_count_for_ms = 82; // counts for how many millis(currently: 82)

// these are checked for in the main program
volatile unsigned long timerCounts;
volatile boolean counterReady;

// internal to counting routine
unsigned long overflowCount;
unsigned int timerTicks;
unsigned int timerPeriod;

void startCounting(unsigned int ms)
{
  counterReady = false; // time not up yet
  timerPeriod = ms;     // how many 1 ms counts to do
  timerTicks = 0;       // reset interrupt counter
  overflowCount = 0;    // no overflows yet

  // reset Timer 1 and Timer 2
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR2A = 0;
  TCCR2B = 0;

  // Timer 1 - counts events on pin D5
  TIMSK1 = bit(TOIE1); // interrupt on Timer 1 overflow

  // Timer 2 - gives us our 1 ms counting interval
  // 16 MHz clock (62.5 ns per tick) - prescaled by 128
  //  counter increments every 8 µs.
  // So we count 125 of them, giving exactly 1000 µs (1 ms)
  TCCR2A = bit(WGM21); // CTC mode
  OCR2A = 124;         // count up to 125  (zero relative!!!!) // def = 124

  // Timer 2 - interrupt on match (ie. every 1 ms)
  TIMSK2 = bit(OCIE2A); // enable Timer2 Interrupt

  TCNT1 = 0; // Both counters to zero
  TCNT2 = 0;

  // Reset prescalers
  GTCCR = bit(PSRASY); // reset prescaler now
  // start Timer 2
  TCCR2B = bit(CS20) | bit(CS22); // prescaler of 128
  // start Timer 1
  // External clock source on T1 pin (D5). Clock on rising edge.
  TCCR1B = bit(CS10) | bit(CS11) | bit(CS12);
} // end of startCounting

ISR(TIMER1_OVF_vect)
{
  ++overflowCount; // count number of Counter1 overflows
} // end of TIMER1_OVF_vect

//******************************************************************
//  Timer2 Interrupt Service is invoked by hardware Timer 2 every 1 ms = 1000 Hz
//  16Mhz / 128 / 125 = 1000 Hz

ISR(TIMER2_COMPA_vect)
{
  // grab counter value before it changes any more
  unsigned int timer1CounterValue;
  timer1CounterValue = TCNT1; // see datasheet, page 117 (accessing 16-bit registers)
  unsigned long overflowCopy = overflowCount;

  // see if we have reached timing period
  if (++timerTicks < timerPeriod)
    return; // not yet

  // if just missed an overflow
  if ((TIFR1 & bit(TOV1)) && timer1CounterValue < 256)
    overflowCopy++;

  // end of gate time, measurement ready

  TCCR1A = 0; // stop timer 1
  TCCR1B = 0;

  TCCR2A = 0; // stop timer 2
  TCCR2B = 0;

  TIMSK1 = 0; // disable Timer1 Interrupt
  TIMSK2 = 0; // disable Timer2 Interrupt

  // calculate total count
  timerCounts = (overflowCopy << 16) + timer1CounterValue; // each overflow is 65536 more
  counterReady = true;                                     // set global flag for end count period
} // end of TIMER2_COMPA_vect

void setup()
{
  // initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Frequency");
  lcd.setCursor(0, 1);
  lcd.print("Counter v0.1b");
  delay(1000);
}

void loop()
{
  while (1)
  {
    PORTB |= _BV(PB4); // For benchmarking purposes
    // ...................do stuff here ....................
    //pinMode(10,OUTPUT);

    // stop Timer 0 interrupts from throwing the count out
    byte oldTCCR0A = TCCR0A;
    byte oldTCCR0B = TCCR0B;
    TCCR0A = 0; // stop timer 0
    TCCR0B = 0;

    startCounting(timer_count_for_ms); // 42 //how many ms to count for //def 500

    while (!counterReady)
    {
    } // loop until count over

    // adjust counts by counting interval to give frequency in Hz
    float frq = (timerCounts * 1000.0) / timer_count_for_ms; // / timerPeriod;

    // Clear LCD for new o/p
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Frequency: ");
    lcd.setCursor(0, 1);
    lcd.println((unsigned long)frq);
    lcd.setCursor(7, 1);
    lcd.println(" Hz ");

    // restart timer 0
    TCCR0A = oldTCCR0A;
    TCCR0B = oldTCCR0B;
  }
} // end of loop