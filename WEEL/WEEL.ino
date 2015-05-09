/**
 * The WEEL @ WE-Festival 2015
 * Vivid Electronics Workshop
 * 
 * License: BSD 3-Clause
 * 
 * Nick Schwarzenberg,
 * 05/2015, v0.1.2
 */




/* :: Dependencies :: */


// include DMX library
#include <LeoDMX.class.h>

// get DMX library instance with default configuration
LeoDMX DMX;




/* :: Configuration :: */


// touch voltage threshold (0…255)
const unsigned char touchThreshold = 8;

// debouncing/blocking time for touch events (millis)
const unsigned short touchDelay = 200;

// startup time after touch (millis)
const unsigned short touchStartupTime = 4000;

// maximum startup spot brightness (0…255)
const unsigned char startupBrightness = 110;

// touch sensor pin
const unsigned short touchPin = A5;

// rotor & blacklight relais pins
const unsigned short rotorPin = 8;
const unsigned short lampPin = 9;




/* :: Global Variables :: */


// touch happening?
bool touching = false;

// timestamp of touch event (large number in millis)
unsigned long touchEventTime;

// current frame for DMX spot
unsigned char dmxFrame[4];




/* :: Main Functions :: */


// initial setup routine
void setup()
{
    // start communication to computer
    Serial.begin( 115200 );
    
    // reset DMX spot
    DMX.null();
    
    // set touch pin as input
    pinMode( touchPin, INPUT );
    
    // set relais pins as outputs
    pinMode( rotorPin, OUTPUT );
    pinMode( lampPin, OUTPUT );
    
    // reset relais pins (idle = high)
    digitalWrite( rotorPin, HIGH );
    digitalWrite( lampPin, HIGH );
}

// continuous loop with main sensor logic
void loop()
{
    // measure touch sensor voltage
    unsigned char currentTouchVoltage = analogRead( touchPin );
    
    // are people currently touching?
    if ( currentTouchVoltage > touchThreshold )
    {
        // touch not yet registered?
        if ( touching == false )
        {
            // remember touch
            touching = true;
            touchEventTime = millis();
            
            // report to host
            Serial.println( "Touch!" );
        }
        
        // calculate elapsed time since touch
        unsigned long touchDuration = millis() - touchEventTime;
        
        // touch just happened shortly?
        if ( touchDuration < touchStartupTime )
        {
            // but not too shortly? (wait for debouncing)
            if ( touchDuration > touchDelay )
            {
                // yea, still starting up
                showStartup( touchDuration );
            }
        }
        else
        {
            // we're turning!
            showTurning();
        }
    }
    // no, no one touching
    else
    {
        // touch still registered?
        if ( touching == true )
        {
            // forget touch
            touching = false;
            
            // report to host
            Serial.println( "Release." );
            
            // render idle mode
            showIdle();
            
            // block for a moment (to protect relais)
            delay( touchDelay );
        }
    }
}




/* :: Action Functions :: */


// idle mode
void showIdle()
{
    // blacklight on (relais inverted!)
    digitalWrite( lampPin, LOW );
    
    // rotor off (relais inverted!)
    digitalWrite( rotorPin, HIGH );
    
    // spot off
    clearFrame( dmxFrame );
    DMX.send( dmxFrame, 4 );
}

// make startup transition
void showStartup( unsigned long elapsedTime )
{
    // lamps off (relais inverted!)
    digitalWrite( lampPin, HIGH );
    
    // rotor on (relais inverted!)
    digitalWrite( rotorPin, LOW );
    
    // calculate progress in animation
    float progress = elapsedTime / (float)touchStartupTime;
    
    // calculate brightness
    unsigned char brightness = char( startupBrightness * progress );
    
    // make red DMX frame
    clearFrame( dmxFrame );
    dmxFrame[0] = brightness;
    
    // send frame to spot
    DMX.send( dmxFrame, 4 );
}

// manage turning animation
void showTurning()
{
    // flash
    dmxFrame[0] = 255;
    dmxFrame[1] = 20;
    dmxFrame[2] = 0;
    dmxFrame[3] = 8;
    DMX.send( dmxFrame, 4 );
    delay( 10 );
    
    // black out
    clearFrame( dmxFrame );
    DMX.send( dmxFrame, 4 );
    delay( 70 );
}

// utility: clear DMX frame
void clearFrame( unsigned char* frame )
{
    // set all channels to zero
    frame[0] = 0;
    frame[1] = 0;
    frame[2] = 0;
    frame[3] = 0;
}
