#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <motors.h>
#include <camera/po8030.h>



#include <process_image.h>
#include <football_game.h>
#include <sensors/proximity.h>




messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);


int main(void)
{
    halInit();
    chSysInit();
    mpu_init();
    //start the USB communication
    usb_start();
    //starts the camera
    dcmi_start();
	po8030_start();
	//initialize the motors
	motors_init();
	//stars the threads for the pi regulator and the processing of the image
	messagebus_init(&bus, &bus_lock, &bus_condvar);

	//start the camera
	process_image_start();
	//create the proximity thread
	proximity_start();
	//create the main thread for the movement of the robot
	football_game_start();


    /* Infinite loop. */
    while (1) {
    	//waits 1 second
        chThdSleepMilliseconds(1000);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
