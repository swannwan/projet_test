#include "ch.h"
#include "hal.h"
#include <sensors/proximity.h>
#include <motors.h>
#include <chprintf.h>
#include <usbcfg.h>


#define DISTANCE_LIM 200

uint16_t angle_to_camera[4] = {70, 161, 325, 523};


/*uint8_t get_angle_to_camera(uint8_t IR_number){

uint8_t angle_to_camera[NUMBER_OF_ANGLE];

	angle_to_camera[0] = 5;
	angle_to_camera[1] = 45;
	angle_to_camera[2] = 90;
	angle_to_camera[3] = 160;
	angle_to_camera[4] = 200;
	//angle_to_camera[5] = 270;
	////angle_to_camera[6] = 300;
	//angle_to_camera[7] = 340;

	return angle_to_camera[IR_number];

};
*/

static THD_WORKING_AREA(waManageDistance, 256);
static THD_FUNCTION(ManageDistance, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    while(1){
        time = chVTGetSystemTime();

        for(uint8_t i = 0; i < 7; i++)
        {
        	if(i==0){
        	if(get_prox(i) > DISTANCE_LIM){

    			left_motor_set_pos (0);
    			right_motor_set_pos(0);

    			left_motor_set_speed (200);
    			right_motor_set_speed (-200);

    			while (left_motor_get_pos() < angle_to_camera[i])
    			{}
    			left_motor_set_speed (0);
    			right_motor_set_speed (0);
        	}

        	if(i==2){
        	if(get_prox(i) > DISTANCE_LIM){

    			left_motor_set_pos (0);
    			right_motor_set_pos(0);

    			left_motor_set_speed (200);
    			right_motor_set_speed (-200);

    			while (left_motor_get_pos() < angle_to_camera[i])
    			{}
    			left_motor_set_speed (0);
    			right_motor_set_speed (0);
        	}



        	}

        		chprintf((BaseSequentialStream *)&SDU1, "captureprox=%d\n", i);
        		chprintf((BaseSequentialStream *)&SDU1, "captureprox=%d\n", get_prox(i));
        		chprintf((BaseSequentialStream *)&SDU1, "captureprox=%d\n", left_motor_get_pos());
        	}
        }
    chThdSleepUntilWindowed(time, time + MS2ST(10));
}


void manage_distance_start(void){
	chThdCreateStatic(waManageDistance, sizeof(waManageDistance), NORMALPRIO, ManageDistance, NULL);
}
