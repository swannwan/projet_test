
// TROUVER LE NOMBRE DE MOTOR STEP POUR QUE LE ROBOT FASSE 1 TOUR COMPLET SUR LUI MEME

#include "ch.h"
#include "hal.h"


#include <sensors/proximity.h>
#include <motors.h>
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <process_image.h>

#define DISTANCE_LIM 				500

#define DISTANCE_LIM_FRNT_SENSOR 	480 // VALUE TO CALIBRATE EXPERIMENTALY

#define MOTOR_STEP_DELTA 			20
#define MOTOR_STEP_COMPLET_ROTATION	1300
// constant declaration **************************************************



static int16_t angle_to_camera[8] = {-5, -30, -90, -130, 130, 90, 30, 5} ; // CHANGE 16 TO 32 IN MOTOR STEP CA SERAIT MIEUX IL FAUT LE TROUVER EXPERIMENTALEMENT






static int16_t color = BLACK_CARD;

static _Bool rotation_clockwise = false;

static int16_t left_motor_step_to_align = 0; // number of motor until the robot is align to the wall CHANGE 32 TO 16
static int16_t left_motor_step_to_bounce = 0;// number of motor until the robot is in the right direction after bounce CHANGE 32 TO 16


//**********************************************************************




// if the proximity sensor found a too close object it give back the value of the angle to camera sensor
int16_t alert_proximity_sensor(void) //CHANGE 16 TO 32
{
	for (int i = 0 ; i < 8 ; i++)
	{
		if (get_prox(i) > DISTANCE_LIM && i != 3 && i !=4) // NE PREND QUE LES CAPTEURS DE DEVANT
			return angle_to_camera[i];
	}
	return 0;
}


void stop_robot (void)
{
	left_motor_set_speed (0);
	right_motor_set_speed (0);
}

void start_robot(void)
{
	left_motor_set_speed (ROBOT_SPEED);
	right_motor_set_speed (ROBOT_SPEED);
}

void game_over(void)
{
	stop_robot();
	systime_t time;
	time = chVTGetSystemTime();

	while (time +  MS2ST(10000) < chVTGetSystemTime()) // !!!!!!!!!!! PAS SUR QUE CETTE FOCNITON SOIT JUSTE
														// FAIT TOURNER LE ROBOT SUR LUI MEME PENDANT 10 SECONDE
	{
		left_motor_set_speed (200);
		right_motor_set_speed (-200);
	}
	stop_robot();
}


// rotate the robot with motor step
// positive motor_step = clockwise rotation
// negative motor_step = anti clockwise rotation

void rotate_robot(int16_t left_motor_step) // CHANGE TO 32
{
	//	stop the robot before making the rotation
	stop_robot();
	// initiate  robot position counter
	left_motor_set_pos (0);
	right_motor_set_pos(0);
	// the rotation depends direction depends on the angle given
	if (left_motor_step > 0)
	{
		rotation_clockwise = true;
		left_motor_set_speed (200);
		right_motor_set_speed (-200);
	}else
	{
		rotation_clockwise = false;
		left_motor_set_speed (-200);
		right_motor_set_speed (200);
	}
//	get motor position in uint32 so we need angle in uint32
	if (rotation_clockwise)
	{
		while (left_motor_get_pos() < left_motor_step){
			//chThdSleepMilliseconds(10);
		}
	}else
	{
		while (left_motor_get_pos() < - left_motor_step){
			//chThdSleepMilliseconds(10);
		}
	}
//	stop the robot after making the rotation
	stop_robot();

}




void rotate_robot_until_align (int16_t angle) //CHANGE TO 32
{
	stop_robot();


	left_motor_set_pos (0);
	right_motor_set_pos(0);

	// the rotation depends direction depends on the angle given
	if (angle < 0)
	{
		rotation_clockwise = true;
		left_motor_set_speed (200);
		right_motor_set_speed (-200);
	}else
	{
		rotation_clockwise = false;
		left_motor_set_speed (-200);
		right_motor_set_speed (200);
	}

	//	get motor position in uint32 so we need angle in uint32
	while (get_prox(0) < DISTANCE_LIM_FRNT_SENSOR  || get_prox(7) < DISTANCE_LIM_FRNT_SENSOR)
	//while (get_prox(0) < DISTANCE_LIM_FRNT_SENSOR)
	{
		//chprintf((BaseSequentialStream *)&SDU1, "captureprox=%d\n", get_prox(0));
		chThdSleepMilliseconds(10);
	}

	left_motor_step_to_align = left_motor_get_pos();

	//	stop the robot after making the rotation
	stop_robot();

}

void rotate_robot_bounce(void)
{
	//	stop the robot before making the rotation
	stop_robot();
	// initiate  robot position counter
	left_motor_set_pos (0);
	right_motor_set_pos(0);
	// the rotation direction depends on the angle given


	// 1ER CONDITION QUI DONNE L'ANGLE POUR UN REBOND PARFAIT

//	if (color != BLUE_CARD)
//	{

		if (left_motor_step_to_align > 0) // SI LA 1ER ROT SE FAIT DANS LE SENS HORAIRE
			left_motor_step_to_bounce = - MOTOR_STEP_COMPLET_ROTATION/2 + left_motor_step_to_align; //ADD COLOR
		else// SI LA 1ER ROT SE FAITR DANS LE SENS ANTIHORAIRE
			left_motor_step_to_bounce = MOTOR_STEP_COMPLET_ROTATION/2 - left_motor_step_to_align; //ADD COLOR

		rotate_robot(left_motor_step_to_bounce);


//	}else{
//		game_over(); //IL FauT CRéER UNE FONCTION de FIN DE JEU
//	}

	//rotate_robot(left_motor_step_to_bounce);
	// CETTE CONDITION REDONNE UN ANGLE EN FONCTION DE LA VUE PAR LE ROBOT
		//	switch (get_color()){	​
		//	    case RED_CARD:
		//	      break;
		//	}

	// CALCUL LES MOTORS STEP POUR REMETTRE LE ROBOT DANS LE BON ANGLE

	//	get motor position in uint32 so we need angle in uint32
	//	stop the robot after making the rotation
	stop_robot();
}

//int32_t angle_into_motor_step (int16_t angle)
//{
//	return angle_convertion;
//}



static THD_WORKING_AREA(waManageDistance, 1024);
static THD_FUNCTION(ManageDistance, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    //rotation_360();
    // systime_t time;

    start_robot();



    while(1){

    	if (alert_proximity_sensor()) // alert proximity return an angle
    	{
    		rotate_robot_until_align(alert_proximity_sensor());
    		//color = get_color();
    		//    		// ON A MTN LE SENS DE LA ROTATION, LA COULEUR DE LA CARTE, ET LES MOTORS STEP POUR L'ANGLE AVEC LE MUR
    		//    		//	ON DOIT FAIRE UNE FONCTION QUI CALCULE UN NOUVELLE ANGLE DE ROTATION (EN MOTOR STEP) ET QUI ROTATE LE ROBOT JUSQU'A CELUI CI
    		rotate_robot_bounce();

    		//
    		start_robot();
    		chThdSleepMilliseconds(1000);
    	}

    	chThdSleepMilliseconds(10);


    }

}


void manage_distance_start(void){
	chThdCreateStatic(waManageDistance, sizeof(waManageDistance), NORMALPRIO, ManageDistance, NULL);
}


//void rotation_360(void){
//
//	left_motor_set_pos (0);
//	right_motor_set_pos(0);
//
//	while(left_motor_get_pos()< 1300){
//
//		//chprintf((BaseSequentialStream *)&SDU1, "MOTOR_POS=%d\n", left_motor_get_pos());
//		left_motor_set_speed (200);
//		right_motor_set_speed (-200);
//
//
//	}
//
//	stop_robot();
//
//}

