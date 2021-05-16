#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"
#include <sensors/proximity.h>

//constants for the different parts of the project
#define IMAGE_BUFFER_SIZE		640



// color define for the game, those value correspond to the motor step variation (0 = BLACK_CARD for a perfect rebound)
#define RED_CARD				-120 	// LA BALLE VA ETRE STOPPER DANS SON REBOND
#define GREEN_CARD				120		// LA BALLE LONGE PLUS LA SURFACE
#define BLUE_CARD				100
#define BLACK_CARD				0


#define DISTANCE_LIM 				500	// minimal distance between the robot sensor and the wall (used to give the alert)
#define DISTANCE_LIM_FRNT_SENSOR 	400 // minimal distance for the two front sensor (TO CALIBRATE EXPERIMENTALY)
#define DISTANCE_DIFF_FRNT_SENSOR 	50 	// distance difference between the two front sensor

// motor step for fundamental angle
#define MOTOR_STEP_180_ROTATION	650
#define MOTOR_STEP_100_ROTATION 361

// speed of the robot in [step/s]
#define ROBOT_SPEED				400
#define ROBOT_ROTATION_SPEED	200

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

void SendUint8ToComputer(uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
