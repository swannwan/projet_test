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



// COLOR USED FOR THE GAME THE NUMBER CORRESPEND TO THE ANGLE SHIFT THAT THERE WILL BE
#define RED_CARD				-120 // LA BALLE VA ETRE STOPPER DANS SON REBOND
#define GREEN_CARD				120	// LA BALLE LONGE PLUS LA SURFACE
#define BLUE_CARD				100
#define BLACK_CARD				0



/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

void SendUint8ToComputer(uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
