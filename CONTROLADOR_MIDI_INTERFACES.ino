///////////////////////////////////////////////////////////////////////////////////////////
// Name : MIDI_Controller
// Written by:   Michael Balzer
// Modified by:  César Darío García Jiménez 
// Fecha: 02/05/2013
// Versión 1.0
//
//     Description:  This program is designed to work with 6 buttons and 5 potentiometers as input, the program sends the values as MIDI 
//                   messages usefull to work with audio DAWs or VST's.
//                   If your hardware doesn't have a midi output setup you can use a MIDI emulator like ""hairless MIDI <-> Serial Bridge"
//                   to parse the serial signal from the arduino using the regular usb conector.
//                   (midi emulator site: http://projectgus.github.com/hairless-midiserial/)
//
// This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// See http://creativecommons.org/licenses/by-nc-sa/3.0/ for license details.
///////////////////////////////////////////////////////////////////////////////////////////

#include <MIDI.h>
#include <SoftwareSerial.h>

// MIDI mapping taken from http://www.nortonmusic.com/midi_cc.html
#define MIDI_CC_MODULATION 0x01
#define MIDI_CC_BREATH 0x02
#define MIDI_CC_VOLUME 0x07
#define MIDI_CC_BALANCE 0x08
#define MIDI_CC_PAN 0x0A
#define MIDI_CC_EXPRESSION 0x0B
#define MIDI_CC_EFFECT1 0x0C
#define MIDI_CC_EFFECT2 0x0D

#define MIDI_CC_GENERAL1 0x0E
#define MIDI_CC_GENERAL2 0x0F
#define MIDI_CC_GENERAL3 0x10
#define MIDI_CC_GENERAL4 0x11
#define MIDI_CC_GENERAL5 0x12
#define MIDI_CC_GENERAL6 0x13
#define MIDI_CC_GENERAL7 0x14
#define MIDI_CC_GENERAL8 0x15
#define MIDI_CC_GENERAL9 0x16
#define MIDI_CC_GENERAL10 0x17
#define MIDI_CC_GENERAL11 0x18
#define MIDI_CC_GENERAL12 0x19
#define MIDI_CC_GENERAL13 0x1A
#define MIDI_CC_GENERAL14 0x1B
#define MIDI_CC_GENERAL15 0x1C
#define MIDI_CC_GENERAL16 0x1D
#define MIDI_CC_GENERAL17 0x1E
#define MIDI_CC_GENERAL18 0x1F

#define MIDI_CC_GENERAL1_FINE 0x2E
#define MIDI_CC_GENERAL2_FINE 0x2F
#define MIDI_CC_GENERAL3_FINE 0x30
#define MIDI_CC_GENERAL4_FINE 0x31
#define MIDI_CC_GENERAL5_FINE 0x32
#define MIDI_CC_GENERAL6_FINE 0x33
#define MIDI_CC_GENERAL7_FINE 0x34
#define MIDI_CC_GENERAL8_FINE 0x35
#define MIDI_CC_GENERAL9_FINE 0x36
#define MIDI_CC_GENERAL10_FINE 0x37
#define MIDI_CC_GENERAL11_FINE 0x38
#define MIDI_CC_GENERAL12_FINE 0x39
#define MIDI_CC_GENERAL13_FINE 0x3A
#define MIDI_CC_GENERAL14_FINE 0x3B
#define MIDI_CC_GENERAL15_FINE 0x3C
#define MIDI_CC_GENERAL16_FINE 0x3D
#define MIDI_CC_GENERAL17_FINE 0x3E
#define MIDI_CC_GENERAL18_FINE 0x3F

#define MIDI_CC_SUSTAIN 0x40
#define MIDI_CC_REVERB 0x5B
#define MIDI_CC_CHORUS 0x5D
#define MIDI_CC_CONTROL_OFF 0x79
#define MIDI_CC_NOTES_OFF 0x78

#define NOTE_C0 0x00 // 0
#define NOTE_C1 0x12 // 18
#define NOTE_C2 0x24 // 36

//#define FASTADC
// defines for setting and clearing register bits
#define FASTADC 1
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Define the number of digital and analog inputs
#define NUM_DI 6
#define NUM_AI 5any

// Define Channel 3 as our communicacion path (it can be any from 0-15) 
#define MIDI_CHANNEL 3
// Define Do central as our starting note 
#define NOTE NOTE_C2
// Define the digital output where our led is goig to be conected.
#define LED_PIN=13;

/*
// The next code define the order of lecture for our digital pins, originally the code was designed for a "midi fighter" model with 12 buttons,
// but I only set 6 buttons so, the original comment that explain this order was this:

  // When mapping to a MIDI Fighter we need to skip a row of buttons. Set this from 0-3 to define which row to skip.
  // Rows are ordered from bottom to top (same as the MIDI Fighter's button layout).
  #define SKIP_ROW 2
  // This pin order corresponds to the bottom left button being zero, increasing by one as we move from left to right, bottom to top
  // 8  9 10 11
  // 4  5  6  7
  // 0  1  2  3
  // This array size must match NUM_DI above.
  #define DIGITAL_PIN_ORDER 10, 11, 12, 13, 6, 7, 8, 9, 2, 3, 4, 5
*/
#define DIGITAL_PIN_ORDER 5,6,7,2,3,4
// Same for analog inputs.
#define ANALOGUE_PIN_ORDER A0, A1, A2, A3, A4

// Shortcut for sendin the "MIDI_CC_GENERAL1" declared above.
#define MIDI_CC MIDI_CC_GENERAL1

// Comment this line out to disable button debounce logic.
// See http://arduino.cc/en/Tutorial/Debounce on what debouncing is used for.
#define DEBOUNCE
// Debounce time length in milliseconds
#define DEBOUNCE_LENGTH 2

// Comment this line out to disable analogue filtering
#define ANALOGUE_FILTER
// A knob or slider movement must initially exceed this value to be recognised as an input. Note that it is
// for a 7-bit (0-127) MIDI value.
#define FILTER_AMOUNT 3
// Timeout is in microseconds
#define ANALOGUE_INPUT_CHANGE_TIMEOUT 250000

// Array containing a mapping of digital pins to channel index.
byte digitalInputMapping[NUM_DI] = {
  DIGITAL_PIN_ORDER};
// Array containing a mapping of analogue pins to channel index.
byte analogueInputMapping[NUM_AI] = {
  ANALOGUE_PIN_ORDER};

// Contains the current state of the digital inputs.
byte digitalInputs[NUM_DI];
// Contains the current value of the analogue inputs.
byte analogueInputs[NUM_AI];

// Variable to hold temporary digital reads, used for debounce logic.
byte tempDigitalInput;
// Variable to hold temporary analogue values, used for analogue filtering logic.
byte tempAnalogueInput;

// Preallocate the for loop index so we don't keep reallocating it for every program iteration.
byte i = 0;
// Variable to hold difference between current and new analogue input values.
byte digitalOffset = 0;
// This is used as a flag to indicate that an analogue input is changing.
byte analogueDiff = 0;

// Variable para calcular si el potenciometro ha dejado de moverse, y eliminar el ruido que se puede provocar por la impresicion del potenciometro y la distribucion flotante de los 5 v de Arduino
boolean analogueInputChanging[NUM_AI];
// Time the analogue input was last moved
unsigned long analogueInputTimer[NUM_AI];

void setup()
{     

  /* ////////////////////////////////////////////////////////////////////////////////////////////////
   By default, the successive approximation circuitry requires an input clock frequency [ADC clock] between 50 kHz and 200 kHz to get maximum resolution. 
   If a lower resolution than 10 bits is needed, 
   the input clock frequency to the ADC can be higher than 200 kHz to get a higher sample rate.
   The ADC clock is 16 MHz divided by a prescale factor. The prescale is set to 128 (16MHz/128 = 125 KHz) in wiring.c. 
   Since a conversion takes 13 ADC clocks, the sample rate is about 125KHz/13 or 9600 Hz.
   So anyway, setting the prescale to, say, 16, would give a sample rate of 77 KHz. 
   Not sure what kind of resolution you would get though!reference:
   http://www.atmel.com/dyn/resources/prod_documents/DOC2559.PDF
   
   // Taken from http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1208715493/11
   // Iniciamos el cambio de pre-escala para nuestro ATMEGA.
   #ifdef FASTADC
   / set prescale to 16
   sbi(ADCSRA,ADPS2) ;
   cbi(ADCSRA,ADPS1) ;
   cbi(ADCSRA,ADPS0) ;
   #endif
   //////////////////////////////////////////////////////////////////////////////////////////// */

  //MIDI RX OMNI MODE
  MIDI.begin();
  //Despues de Iniciar la comunicacion MIDI, iniciamos la Comunicacion Serial a 115200, requisito que sea de esta forma para trabajar con el software "airless-midi <-> Serial bridge"
  Serial.begin(115200);
  
  // Initialise each digital input channel.
  for (i = 0; i < NUM_DI; i++)
  {
    // Set the pin direction to input.
    pinMode(digitalInputMapping[i], INPUT);
    // Initialise the digital state with a read to the input pin.
    digitalInputs[i] = digitalRead(digitalInputMapping[i]);
  }
  
  // Initialise each analogue input channel.
  for (i = 0; i < NUM_AI; i++)
  {
    // Set the pin direction to input.
    pinMode(analogueInputMapping[i], INPUT);
    // Initialise the analogue value with a read to the input pin.
    analogueInputs[i] = analogRead(analogueInputMapping[i]);
    // Assume no analogue inputs are active
    analogueInputChanging[i] = false;
    analogueInputTimer[i] = 0;
  }  
  pinMode(13, OUTPUT);
}   

void loop()
{
  
  for (i = 0; i < NUM_DI; i++)
  {
    digitalOffset = i;
    // Read the current state of the digital input and store it temporarily.
    tempDigitalInput = digitalRead(digitalInputMapping[i]);
    // Check if the last state is different to the current state.
    if (digitalInputs[i] != tempDigitalInput)
    {
#ifdef DEBOUNCE
      // Wait for a short period of time, and then take a second reading from the input pin.
      delay(DEBOUNCE_LENGTH);
      // If the second reading is the same as the initial reading, assume it must be true.
      if (tempDigitalInput == digitalRead(digitalInputMapping[i]))
      {
#endif
        // Record the new digital input state.
        digitalInputs[i] = tempDigitalInput;
        // Moved from HIGH to LOW (button pressed)
        if (digitalInputs[i] == 1)
        {
          noteOn(MIDI_CHANNEL, NOTE + digitalOffset, 0x7F); // Channel 1, middle C, maximum velocity     
          digitalWrite(13,HIGH);
          delay(15);
          digitalWrite(13,LOW);
        }
        else
           // Moved from LOW to HIGH (button released)
          noteOff(MIDI_CHANNEL, NOTE + digitalOffset); // Channel 1, middle C  
#ifdef DEBOUNCE
      }
#endif
    }
  }
/*
   * Analogue input logic:
   * The Arduino uses a 10-bit (0-1023) analogue to digital converter (ADC) on each of its analogue inputs.
   * The ADC isn't very high resolution, so if a pot is in a position such that the output voltage is 'between'
   * what it can detect (say 2.505V or about 512.5 on a scale of 0-1023) then the value read will constantly
   * fluctuate between two integers (in this case 512 and 513).
   *
   * If we're simply looking for a change in the analogue input value like in the digital case above, then
   * there will be cases where the value is always changing, even though the physical input isn't being moved.
   * This will in turn send out a constant stream of MIDI messages to the connected software which may be problematic.
   *
   * To combat this, we require that the analogue input value must change by a certain threshold amount before
   * we register that it is actually changing. This is good in avoiding a constantly fluctuating value, but has
   * the negative effect of a reduced input resolution. For example if the threshold amount was 2 and we slowly moved
   * a slider through it's full range, we would only detect every second value as a change, in effect reducing the
   * already small 7-bit MIDI value to a 6-bit MIDI value.
   *
   * To get around this problem but still use the threshold logic, a timer is used. Initially the analogue input
   * must exceed the threshold to be detected as an input. Once this occurs, we then read every value coming from the
   * analogue input (not just those exceeding a threshold) giving us full 7-bit resolution. At the same time the
   * timer is started. This timer is used to keep track of whether an input hasn't been moved for a certain time
   * period. If it has been moved, the timer is restarted. If no movement occurs the timer is just left to run. When
   * the timer expires the analogue input is assumed to be no longer moving. Subsequent movements must exceed the
   * threshold amount.
*/
  for (i = 0; i < NUM_AI; i++)
  {
    // Read the analogue input pin, dividing it by 8 so the 10-bit ADC value (0-1023) is converted to a 7-bit MIDI value (0-127).
    tempAnalogueInput = analogRead(analogueInputMapping[i]) / 8;
    // Take the absolute value of the difference between the curent and new values 
    analogueDiff = abs(tempAnalogueInput - analogueInputs[i]);
    // Only continue if the threshold was exceeded, or the input was already changing
    if ((analogueDiff > 0 && analogueInputChanging[i] == true) || analogueDiff >= FILTER_AMOUNT)
    {
      // Only restart the timer if we're sure the input isn't 'between' a value
      // ie. It's moved more than FILTER_AMOUNT
      if (analogueInputChanging[i] == false || analogueDiff > FILTER_AMOUNT)
      {
        // Reset the last time the input was moved
        analogueInputTimer[i] = micros();
       // The analogue input is moving
        analogueInputChanging[i] = true;
      }
      else if (micros() - analogueInputTimer[i] > ANALOGUE_INPUT_CHANGE_TIMEOUT)
      {
        analogueInputChanging[i] = false;
      }
      // Only send data if we know the analogue input is moving
      if (analogueInputChanging[i] == true)
      {
        // Record the new analogue value
        analogueInputs[i] = tempAnalogueInput;
        // Send the analogue value out on the general MIDI CC (see definitions at beginning of this file)
        controlChange(MIDI_CHANNEL, MIDI_CC + i, analogueInputs[i]);
      }
    }
  }
} 


// Send a MIDI note on message
void noteOn(byte channel, byte pitch, byte velocity)
{
  // 0x90 is the first of 16 note on channels. Subtract one to go from MIDI's 1-16 channels to 0-15
  channel += 0x90 - 1;
  // Ensure we're between channels 1 and 16 for a note on message
  if (channel >= 0x90 && channel <= 0x9F)
  {
    Serial.write(channel);
    Serial.write(pitch);
    Serial.write(velocity); 
  }
}

// Send a MIDI note off message
void noteOff(byte channel, byte pitch)
{
  // 0x80 is the first of 16 note off channels. Subtract one to go from MIDI's 1-16 channels to 0-15
  channel += 0x80 - 1;
  // Ensure we're between channels 1 and 16 for a note off message
  if (channel >= 0x80 && channel <= 0x8F)
  {
    Serial.write(channel);
    Serial.write(pitch);
    Serial.write((byte)0x00);
  }
}

// Send a MIDI control change message
void controlChange(byte channel, byte control, byte value)
{
  // 0xB0 is the first of 16 control change channels. Subtract one to go from MIDI's 1-16 channels to 0-15
  channel += 0xB0 - 1;
  // Ensure we're between channels 1 and 16 for a CC message
  if (channel >= 0xB0 && channel <= 0xBF)
  {
    Serial.write(channel);
    Serial.write(control);
    Serial.write(value);
    digitalWrite(13,HIGH);
    delay(5);
    digitalWrite(13,LOW);   
  }
}


