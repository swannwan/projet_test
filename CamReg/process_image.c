
//test

#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>


#include <process_image.h>




//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);


static int16_t color_image = BLACK_CARD;

 // Returns color of the image
 // r = red ; b = blue ; g = green ; n = no color



int16_t extract_image_color(uint8_t *buffer){

	int16_t color;
	uint32_t red_mean = 0;
	uint32_t green_mean = 0;
	uint32_t blue_mean = 0;
	uint32_t total_mean = 0;

	//uint32_t value_test = 20;

	for (uint16_t i = 0; i < (IMAGE_BUFFER_SIZE) ; i++){
		red_mean += buffer[3*i];
		green_mean += buffer[3*i+1];
		blue_mean += buffer[3*i+2];
	}


	red_mean /= IMAGE_BUFFER_SIZE;
	green_mean /= IMAGE_BUFFER_SIZE;
	blue_mean /= IMAGE_BUFFER_SIZE;
	total_mean = (red_mean + green_mean + blue_mean)/3 ;

//	chprintf((BaseSequentialStream *)&SDU1, "red_mean=%d\n", red_mean);
//	chprintf((BaseSequentialStream *)&SDU1, "green_mean=%d\n",green_mean);
//	chprintf((BaseSequentialStream *)&SDU1, "blue_mean=%d\n", blue_mean);

	if ((red_mean > total_mean + 40) && (red_mean > blue_mean) && (red_mean > green_mean)) // function test for color determination
		return color = RED_CARD;

	if ((green_mean > total_mean + 10) && (green_mean > blue_mean) && (green_mean > red_mean))
		return color = GREEN_CARD;

	if ((blue_mean > total_mean + 10) && (blue_mean > red_mean) && (blue_mean > green_mean))
		return color = BLUE_CARD;

	return color = BLACK_CARD;

}





static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();
	po8030_set_ae(1);
	po8030_set_awb(0);

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
    }
}


static THD_WORKING_AREA(waProcessImage, 4096);
static THD_FUNCTION(ProcessImage, arg) {

    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[3*IMAGE_BUFFER_SIZE] = {0};





    while(1){
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);

		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr(); // this function is

		//Extracts all color pixel
		for(uint16_t i = 0 ; i < (IMAGE_BUFFER_SIZE) ; i++){
			//extracts first 5bits of the first byte to have only red

			//the result is shifted to have a value at the start of the bytes
			image[3*i] = (uint8_t)img_buff_ptr[2*i]&0xF8;

			//extract green and place it in image, (green is in between two buffer we have some bit manipulation)
			image[3*i +1] = (((uint8_t)img_buff_ptr[2*i]&0x07)<<5) | (((uint8_t)img_buff_ptr[2*i+1]&0xE0)>>3); // (3 and 5 to have 6bit 64 value)change to 2 and 6 in order to have only 5 bit for the green color

			//extracts first 5bits of the second byte to have only blue
			image[3*i +2] = ((uint8_t)img_buff_ptr[2*i+1]&0x1F) << 3;
		}

		color_image = extract_image_color(image);

		//chprintf((BaseSequentialStream *)&SDU1, "COLOR=%d\n", color_image);

		//converts the width into a distance between the robot and the camera
		/*
		if(send_to_computer){
			//sends to the computer the image
			SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);

		}
		//invert the bool
		send_to_computer = !send_to_computer;*/
    }
}

int16_t get_color(void){
	return color_image;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
