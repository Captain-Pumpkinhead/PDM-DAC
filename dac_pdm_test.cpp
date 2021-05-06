#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"

int main()
{
    // Types of DACs I know about:
    //  - Pulse Width Modulation/Pulse Density Modulation
    //  - R-2R Ladder (Resistor Ladder)
    //  - Weighted Resistor Ladder
    //  - Delta-Sigma (requires comparators)

    //GPIO Pin Constants//////////////////////////////////////
    const uint LED_BLUE  = 0;                               // //The RGB LED pins.  Used to give debug/feedback info.
    const uint LED_GREEN = 1;                               //   Blue visualizes frequency.  Green shows button press.
    const uint LED_RED   = 2;                               //   Red shows clock cycle.
    const uint LED_BOARD = 25;                              // //The Pico's built in LED.
    const uint PDM_OUT   = 28;                              // //Pulse Density Modulation output.  Gonna try 4 of them
    const uint PDM_OUT2  = 27;                              //   and see if that solves my problems. See line 98 (+81)
    const uint PDM_OUT3  = 26;                              //
    const uint PDM_OUT4  = 21;                              //
    const uint BUTTON1   = 4;                               // //Double the frequency
    const uint BUTTON2   = 8;                               // //Divides the frequency by 3
    const uint BUTTON3   = 15;                              // //Double the wait time
    const uint BUTTON4   = 16;                              // //Divides the wait time by 3
    const double PI_x2   = 6.2831853072;                    // //Pi times 2, up to 10 decimal places.
    //////////////////////////////////////////////////////////

    //Variables/////////////////////
    double i1             = 0.000;// //My iteration variable
    int    i2             = 0;    // //iteration for ints
    double increment      = 0.000;// //How much to move i1 forward by each cycle.
    bool button1_pressed  = 0;    // //Whether the button was pressed one frame ago.
    bool button2_pressed  = 0;    //
    bool button3_pressed  = 0;    //
    bool button4_pressed  = 0;    //
    int  sample_frequency = 1;    // //How many microseconds to wait before performing the next iteration.
    double wave_frequency = 1.25; // //How many sine waves to output per second.
    double wave_position  = 0.000;// //This shows the height of the sine wave.  The analog output we're shooting for.
    double pdm_meter      = 0.000;//
    ////////////////////////////////
    
    //GPIO Initialization////////////////// //GPIO stands for General Purpose Input Output.
    gpio_init(LED_BLUE);                 //   If you see PIO, that means Progragrammable Input Output.
    gpio_init(LED_GREEN);                //
    gpio_init(LED_RED);                  //
    gpio_init(LED_BOARD);                //
    gpio_init(PDM_OUT);                  //
    gpio_init(PDM_OUT2);                 //
    gpio_init(PDM_OUT3);                 //
    gpio_init(PDM_OUT4);                 //
    gpio_init(BUTTON1);                  //
    gpio_init(BUTTON2);                  //
    gpio_init(BUTTON3);                  //
    gpio_init(BUTTON4);                  //
    gpio_set_dir(LED_BLUE,  GPIO_OUT);   // //You can't use a pin as both input and output.
    gpio_set_dir(LED_GREEN, GPIO_OUT);   //
    gpio_set_dir(LED_RED,   GPIO_OUT);   //
    gpio_set_dir(LED_BOARD, GPIO_OUT);   //
    gpio_set_dir(PDM_OUT,   GPIO_OUT);   //
    gpio_set_dir(PDM_OUT2,  GPIO_OUT);   //
    gpio_set_dir(PDM_OUT3,  GPIO_OUT);   //
    gpio_set_dir(PDM_OUT4,  GPIO_OUT);   //
                                         //
    gpio_set_dir(BUTTON1, GPIO_IN);      // //The buttons are input, and must be configured as such.
    gpio_set_dir(BUTTON2, GPIO_IN);      //
    gpio_set_dir(BUTTON3, GPIO_IN);      //
    gpio_set_dir(BUTTON4, GPIO_IN);      //
    gpio_pull_up(BUTTON1);               // //Pulling them high lets me connect the other side to ground.  It's easier
    gpio_pull_up(BUTTON2);               //   that connecting it to another pin and having to configure that one too.
    gpio_pull_up(BUTTON3);               //
    gpio_pull_up(BUTTON4);               //
                                         //
    stdio_init_all();                    // //This allows me to perform serial output.  CMakeLists has that set to USB.
    ///////////////////////////////////////

    //Final Prep///////////////////////////////////////////////////////////
    increment = PI_x2 / (1000000/(sample_frequency + 24)/wave_frequency);// //1,000,000/sample = number of ticks per second (ticks)
                                                                         //   ticks/wave = number of ticks per sine wave (resolution)
    ///////////////////////////////////////////////////////////////////////   6.283/resolution = how much to increment to get the right frequency.

    //Code Infinite Loop:
    while (true)
    {
        for(i1=0.000; i1<=6.283; i1+= increment)   //2pi is the repeat point.
        {
            //Create the wave signal, either sine or timbre sine
            wave_position = 1-(sin(i1)/2+0.5); //Standard Sine Wave
            //wave_position = 1-(( sin(i1) + sin(2*i1)/2 + sin(3*i1)/3 + sin(4*i1)/4 + sin(5*i1)/5 + sin(6*i1)/6 + sin(7*i1)/7 + sin(7*i1)/7 )/3.348+0.5); //Timbre Sine Wave, work in progress
            

            //PDM Output/////////////////
            pdm_meter += wave_position;// //Add to b_meter the difference between 1 and sine(i1), assuming the floor is 0 instead of -1
            if (pdm_meter >= 1.000)    // //Once the difference has built up to a value of 1,
            {                          //
                pdm_meter -= 1.000;    // //Reset my counter, but not to 0. The cumulative effect is important.
                gpio_put(PDM_OUT,  0); // //Output a value of 0 to the Analog Converter.
                gpio_put(PDM_OUT2, 0); // //The reason I have multiple PDM ports is to combat range squishing.  See, at higher
                gpio_put(PDM_OUT3, 0); //   frequencies, the peak-to-trough distance gets squished... Adding more pins (current?)
                gpio_put(PDM_OUT4, 0); //   helps stabilize the voltage.  Not necessary if only using lower frequencies, and can
            }                          //   be removed if only using higher frequencies by reducing the capacitence of C1.
            else                       // //Three pins seems to be the sweet spot, where the returns begin to greatly diminish.
            {                          //
                                       //
                gpio_put(PDM_OUT,  1); // //Output a value of 0 to the Analog Converter.
                gpio_put(PDM_OUT2, 1); //
                gpio_put(PDM_OUT3, 1); //
                gpio_put(PDM_OUT4, 1); //
                //gpio_put(LED_BLUE, 1); // //Send it to the LED too, so I have a visual representation.
            }                          //
            /////////////////////////////

            //Button Functions///////////////////////////////////////////////////////////////////////////
            gpio_put(LED_GREEN, 0);                                                                    // //By turning these off here, I can turn them on later without having
            gpio_put(LED_BOARD, 0);                                                                    //   to worry about the other button function turning it on.
                                                                                                       //
            //Button 1///////////////////////////////////////////////////////////////////////////////////
            if (!gpio_get(BUTTON1))                                                                    // //If the button is pressed...
            {                                                                                          //
                if (button1_pressed == 0)                                                              // //Our button_pressed variables ensure that a button press
                {                                                                                      //   is only counted once, and not every microsecond.
                    wave_frequency *= 2;                                                               // //Twice as many sine waves per second
                    increment       = PI_x2 / (1000000/(sample_frequency + 24)/wave_frequency);        // //See line 78
                    gpio_put(LED_BOARD, 1);                                                            // //Light flashes once to confirm that something happened.
                    printf("Wave Frequency: %.2f\n", wave_frequency);                                  //
                    printf("Sample Frequency: %d\n", sample_frequency);                                //
                    printf("Increment: %.10f\n",     increment);                                       //
                    printf("Resolution: %.10f\n",    (1000000/(sample_frequency + 24))/wave_frequency);//
                }                                                                                      //
                button1_pressed = 1;                                                                   //
                gpio_put(LED_GREEN, 1);                                                                //
            }                                                                                          //
            else                                                                                       //
                {button1_pressed = 0;}                                                                 //
                                                                                                       //
            //Button 2///////////////////////////////////////////////////////////////////////////////////
                                                                                                       //
            if (!gpio_get(BUTTON2))                                                                    // //If the button is pressed...
            {                                                                                          //
                if (button2_pressed == 0)                                                              // //Our button_pressed variables ensure that a button press
                {                                                                                      //   is only counted once, and not every microsecond.
                    wave_frequency /= 3;                                                               // //One third as many sine waves per second
                    increment       = PI_x2 / (1000000/(sample_frequency + 24)/wave_frequency);        // //See line 78
                    gpio_put(LED_BOARD, 1);                                                            // //Light flashes once to confirm that something happened.
                    printf("Wave Frequency: %.2f\n", wave_frequency);                                  // //Print to PC the Wave Frequency, Sample Frequency, Increment,
                    printf("Sample Frequency: %d\n", sample_frequency);                                //   and Resolution.
                    printf("Increment: %.10f\n",     increment);                                       //
                    printf("Resolution: %.10f\n",    (1000000/(sample_frequency + 24))/wave_frequency);//
                }                                                                                      //
                button2_pressed = 1;                                                                   //
                gpio_put(LED_GREEN, 1);                                                                //
            }                                                                                          //
            else                                                                                       //
                {button2_pressed = 0;}                                                                 //
                                                                                                       //
            //Button 3///////////////////////////////////////////////////////////////////////////////////
                                                                                                       //
            if (!gpio_get(BUTTON3))                                                                    //
            {                                                                                          //
                if (button3_pressed == 0)                                                              //
                {                                                                                      //
                    sample_frequency *= 2;                                                             // //Wait twice as long as before
                    increment         = PI_x2 / (1000000/(sample_frequency + 24)/wave_frequency);      // //See line 78
                    gpio_put(LED_BOARD, 1);                                                            //
                    printf("Wave Frequency: %.2f\n", wave_frequency);                                  //
                    printf("Sample Frequency: %d\n", sample_frequency);                                //
                    printf("Increment: %.10f\n",     increment);                                       //
                    printf("Resolution: %.10f\n",    (1000000/(sample_frequency + 24))/wave_frequency);//
                }                                                                                      //
                button3_pressed = 1;                                                                   //
                gpio_put(LED_GREEN, 1);                                                                //
            }                                                                                          //
            else                                                                                       //
                {button3_pressed = 0;}                                                                 //
                                                                                                       //
            //Button 4///////////////////////////////////////////////////////////////////////////////////
                                                                                                       //
            if (!gpio_get(BUTTON4))                                                                    //
            {                                                                                          //
                if (button4_pressed == 0)                                                              //
                {                                                                                      //
                    sample_frequency = ceil((double) wave_frequency/3);                                // //Wait one third as long as before
                    increment        = PI_x2 / (1000000/(sample_frequency + 24)/wave_frequency);       // //See line 78
                    gpio_put(LED_BOARD, 1);                                                            //
                    printf("Wave Frequency: %.2f\n", wave_frequency);                                  //
                    printf("Sample Frequency: %d\n", sample_frequency);                                //
                    printf("Increment: %.10f\n",     increment);                                       //
                    printf("Resolution: %.10f\n",    (1000000/(sample_frequency + 24))/wave_frequency);//
                }                                                                                      //
                button4_pressed = 1;                                                                   //
                gpio_put(LED_GREEN, 1);                                                                //
            }                                                                                          //
            else                                                                                       //
                {button4_pressed = 0;}                                                                 //
            /////////////////////////////////////////////////////////////////////////////////////////////

            //Sample Loop End Functions////////
            gpio_put(LED_RED, 0);            // //Measure the sleep time on oscilloscope
            sleep_us(sample_frequency);      // //Wait x microseconds before repeat.  See line 207 (+8) for important comments.
            gpio_put(LED_RED, 1);            // //Measure the program run time on oscilloscope ~24us
            if (i1 + wave_frequency >= PI_x2)// //If we subtract here to make sure that this for loop never ends, then
            {                                //   this allows for smoother transitions at the end of a cycle.  If a sample_frequency
                i1 -= PI_x2;                 //   doesn't offer a smooth end  to the loop, this method allows for that smooth
            }                                //   transition.
            ///////////////////////////////////   Doing this makes me think I shouldn't have used a for loop.  Oh well.

            /* About the sample_frequency wait time:
               After you make your changes, I HIGHLY recommend plugging in a pin to test how long your process takes. Innacurate
               wait times mean your frequency gets thrown out of wack.  A better method for a more integrated system is to use
               a clock (maybe the internal clock, maybe an external quartz clock) to keep the update cycle consistent.

               Anecdotal story:
               This code started as a test to try three different types of Digital to Analog Conversions: Binary Weighted Resistor
               Ladder vs Resistor-2-Resistor Ladder vs Pulse Density Modulation.  I (mistakenly) assumed the computation time per
               cycle was roughly 10 or 20 microseconds, something small, and so I had my wait time set to 1 microsecond, doubled by
               however many times the button was pressed.  But once I actually measured it, it was ~306 microseconds per cycle...
               Big difference, and it was ruining my frequency maths.  Lesson learned, don't assume this stuff, always measure it.
            */

        }//End For() Loop
    }//End While() Loop, which in theory shouldn't really be reached because the for() loop gets extended before it can close...
}//End main()