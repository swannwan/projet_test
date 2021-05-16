#include "ch.h"
#include "hal.h"

#include <sensors/proximity.h>
#include <motors.h>
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <process_image.h>
#include <leds.h>



/*! \brief This is the main file of our project "E-foot"
 * This a game where two person can play each other. The E-PUCK is transform into a ball and will bounce on every obstacle.
 * The player can place red or green card that will change the bounce of the ball. Each player must shoot the E-PUCK into the
 * adversary goal to win.
 *
 * 	@call function : The thread "FootballGame" can be started by the function  football_game_start(void)  include in the .h file
 */


//table that tells us if the sensor is right or left
static _Bool sensor_direction[8] = {1, 1, 1, 1, 0, 0, 0, 0} ; //1 for the right sensor 0 for the left sensor


//if the proximity sensor found a too close object it give back the number of the IR sensor who detected it
uint8_t alert_proximity_sensor(void)
{
	for (uint8_t i = 0 ; i < 8 ; i++)
	{
		if (get_prox(i) > DISTANCE_LIM && i != 3 && i !=4) //don't take back sensor
			return i+1; //+1 because we don't want it to return 0
	}
	return 0;
}

//set the robot speed to zero
void stop_robot (void)
{
	left_motor_set_speed (0);
	right_motor_set_speed (0);
}

//set the robot speed to a constant value
void start_robot(void)
{
	left_motor_set_speed (ROBOT_SPEED);
	right_motor_set_speed (ROBOT_SPEED);
}

//the robot rotate right at a constant speed which is define by ROBOT_ROTATION_SPEED
void right_rotation (void)
{
	left_motor_set_speed (ROBOT_ROTATION_SPEED);
	right_motor_set_speed (-ROBOT_ROTATION_SPEED);
}

//the robot rotate left at a constant speed which is define by ROBOT_ROTATION_SPEED
void left_rotation (void)
{
	left_motor_set_speed (-ROBOT_ROTATION_SPEED);
	right_motor_set_speed (ROBOT_ROTATION_SPEED);
}

//function for the end game animation
void game_over(void)
{
	stop_robot();
	systime_t time;
	systime_t time_led;
	_Bool toggle_led = 0;

	time = chVTGetSystemTime();
	while (time +  MS2ST(10000) > chVTGetSystemTime()) //10 second animation
	{

		time_led = chVTGetSystemTime();

		while (time_led +  MS2ST(500) > chVTGetSystemTime()) //alternation between rotation and stop
		{
			if (toggle_led)
			{
				for(uint8_t i = 0 ; i < 4 ; i++){
					set_led(i, 2);
				}
				right_rotation();
			}
			else
			{
				for(uint8_t i = 0 ; i < 4 ; i++){
					set_led(i, 0);
				}
				stop_robot();
			}
		}
		toggle_led = !(toggle_led);
	}
	stop_robot();

	for(uint8_t i = 0 ; i < 4 ; i++){
		set_led(i, 0);
	}
	stop_robot();
}

//rotate the robot for a given left motor step
void rotate_robot(int32_t left_motor_step)
{
	//if left_motor_step > 0  => clockwise rotation
	//if left_motor_step < 0  => counterclockwise rotation

	//stop the robot before making the rotation
	stop_robot();

	//initiate  robot position counter
	left_motor_set_pos (0);

	//the rotation depends direction depends on the angle given
	if (left_motor_step > 0)
		right_rotation();
	else
		left_rotation();

	//left_motor_get_pos is in uint32 so we need left_motor_step in uint32
	if (left_motor_step > 0)
	{
		//waiting loop
		while (left_motor_get_pos() < left_motor_step)
			chThdSleepMilliseconds(10);
	}else
	{
		//waiting loop
		while (left_motor_get_pos() > left_motor_step)
			chThdSleepMilliseconds(10);
	}
	//stop the robot after making the rotation
	stop_robot();
}

//function that align the robot perpendicular to the wall using only the IR sensor
int32_t rotate_robot_until_align (_Bool direction) //direction = 1 for a right turn , = 0 for a left turn
{

	stop_robot();

	//create variable
	int32_t left_motor_step_to_align = 0;

	//set the motor position counter to zero
	left_motor_set_pos (0);

	//rotate robot
	if (direction)
		right_rotation();
	else
		left_rotation();

	//condition of alignment
	while (get_prox(0) < DISTANCE_LIM_FRNT_SENSOR || abs(get_prox(7) - get_prox(0)) > DISTANCE_DIFF_FRNT_SENSOR)
	{
		chThdSleepMilliseconds(10);
	}

	//stop the motors after making the rotation
	stop_robot();

	//save the value of the first rotation
	left_motor_step_to_align = left_motor_get_pos();

	return left_motor_step_to_align;
}

void rotate_robot_bounce(int32_t left_motor_step_to_align, int32_t left_motor_step_to_bounce, int16_t color)
{
	//stop the robot before making the rotation
	stop_robot();
	//initiate  robot position counter
	left_motor_set_pos (0);

	//the rotation direction depends on the angle given
	//find the motor step for a perfect bounce (like reflection)
	if (left_motor_step_to_align > 0)
	{ 	//if the 1st rotation clockwise
		left_motor_step_to_bounce = - MOTOR_STEP_180_ROTATION + left_motor_step_to_align  + (int32_t)color ;
			if (left_motor_step_to_bounce > -MOTOR_STEP_100_ROTATION)
				left_motor_step_to_bounce = -MOTOR_STEP_100_ROTATION; //condition to be sure the robot don't drive into wall (this can be caused by the card color)
	}
	else
	{	//if the 1st rotation counterclockwise
		left_motor_step_to_bounce = MOTOR_STEP_180_ROTATION + left_motor_step_to_align - (int32_t)color ;
			if (left_motor_step_to_bounce < MOTOR_STEP_100_ROTATION)
				left_motor_step_to_bounce = MOTOR_STEP_100_ROTATION; //condition to be sure the robot don't drive into wall
	}
	rotate_robot(left_motor_step_to_bounce);

	//stop the robot after making the rotation
	stop_robot();
}

//principal thread for the game
static THD_WORKING_AREA(waFootballGame, 1024);
static THD_FUNCTION(FootballGame, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    //number of the sensor (1 to 8)
    uint8_t sensor_number = 0;
    int16_t card_color = BLACK_CARD;

    //variables for the rotations
    int32_t first_rotation = 0; //number of motor step (for the left motor) until the robot is align to the wall
    int32_t second_rotation = 0;//number of motor step (for the left motor) until the robot is in the right direction after bounce

    start_robot();

    while(1)
    {
    	sensor_number = alert_proximity_sensor();

    	if (sensor_number) //if sensor_number = 0 there is no obstacle near
    	{
    		//the value of the alignment rotation is saved
    		first_rotation = rotate_robot_until_align(sensor_direction[sensor_number]);

    		card_color = get_color();

    		if (card_color == BLUE_CARD) //blue card => goal
    			game_over();
    		else
    			rotate_robot_bounce(first_rotation, second_rotation, card_color);
    		start_robot();

    		chThdSleepMilliseconds(400);
    	}
    	chThdSleepMilliseconds(10);
    }

}

void football_game_start(void){
	chThdCreateStatic(waFootballGame, sizeof(waFootballGame), NORMALPRIO, FootballGame, NULL);
}
