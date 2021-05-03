#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"
#include <sensors/proximity.h>

//constants for the difFerents parts of the project
#define IMAGE_BUFFER_SIZE		640
#define ROBOT_SPEED				400

// COLOR USED FOR THE GAME THE NUMBER CORRESPEND TO THE ANGLE SHIFT THAT THERE WILL BE
#define RED_CARD				-40 // LA BALLE VA ETRE STOPPER DANS SON REBOND
#define GREEN_CARD				40	// LA BALLE LONGE PLUS LA SURFACE
#define BLUE_CARD				100
#define BLACK_CARD				0




/*
#define WIDTH_SLOPE				5
#define MIN_LINE_WIDTH			40
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			2
#define PXTOCM					1570.0f //experimental value
#define GOAL_DISTANCE 			10.0f
#define MAX_DISTANCE 			25.0f
#define ERROR_THRESHOLD			0.1f	//[cm] because of the noise of the camera
#define KP						800.0f
#define KI 						3.5f	//must not be zero
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT/KI)
*/



/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

void SendUint8ToComputer(uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
