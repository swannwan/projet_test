// must be renamed used for some test camera detection color



#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>

//simple PI regulator implementation


static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    char color = 'n';
    int16_t speed = 0;


    while(1){
        time = chVTGetSystemTime();

        //computes the speed to give to the motors
        color = get_color();


       /* switch (color)
        ​{
            case 'r':
              speed = -500;
              break;

            case 'n':
              speed = 500;
              break;

            default:
              // default statements
        }*/

        if (color == 'n')
        	speed = 500;
        if (color == 'r')
        	speed = -500;

        //applies the speed from the PI regulator and the correction for the rotation
		right_motor_set_speed(speed);
		left_motor_set_speed(speed);

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}
