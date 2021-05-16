#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>


#include <process_image.h>


static int16_t color_image = BLACK_CARD;

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);


/*
 * Returns color of the image
 * 	RED_CARD = -120 ; GREEN_CARD = 120  ; BLUE_CARD = 100 ; BLACK_CARD = 0
 */
int16_t extract_image_color(uint8_t *buffer){

	int16_t color;
	uint32_t red_mean = 0, green_mean = 0, blue_mean = 0, total_mean = 0;

	//performs an average
	for (uint16_t i = 0; i < (IMAGE_BUFFER_SIZE) ; i++){
		red_mean += buffer[3*i];
		green_mean += buffer[3*i+1];
		blue_mean += buffer[3*i+2];
	}

	red_mean /= 2 * IMAGE_BUFFER_SIZE; //the factor 2 is for the calibration (red is too sensitive)
	green_mean /= IMAGE_BUFFER_SIZE;
	blue_mean /= IMAGE_BUFFER_SIZE;

	total_mean = (red_mean + green_mean + blue_mean)/3 ;

	// color determination
	if ((red_mean > total_mean + 30) && (red_mean > blue_mean) && (red_mean > green_mean))
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
	//enable auto exposure
	po8030_set_ae(1);
	//disable auto white balance
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

			//extracts first 5bits of the first byte to have red
			image[3*i] = (uint8_t)img_buff_ptr[2*i]&0xF8;

			//extract the green part and place it in image, (green is in between two buffer we have some bit manipulation)
			image[3*i +1] = (((uint8_t)img_buff_ptr[2*i]&0x07)<<5) | (((uint8_t)img_buff_ptr[2*i+1]&0xE0)>>3);

			//extracts first 5bits of the second buffer to have only blue
			image[3*i +2] = ((uint8_t)img_buff_ptr[2*i+1]&0x1F) << 3;
		}
		color_image = extract_image_color(image);

    }
}

int16_t get_color(void){
	return color_image;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}

