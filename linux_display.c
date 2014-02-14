/*
ECE 468 - Lab 1
Written By: Taylor Steinberg
Instructor: Adam Hoover
Date: Jan 22, 2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

int main(int argc, char **argv) {

	Display *Monitor;
	Window ImageWindow;
	GC ImageGC;
	XWindowAttributes Atts;
	XImage *Picture;

	int ROWS;
	int COLS;
	unsigned char *displaydata;

	char colorCode[3];
	char maxColorVal[4];

	// Check input paramters
	if(argc != 2) {
		printf("Invalid input paramters\n");
		return 0;
	}

	// Open file pointer
	FILE *fp = NULL;
	fp = fopen(argv[1], "rb");

	if(fp == NULL) {
		printf("Unable to open file\n");
		return 0;
	}

	// Read in file header
	int err = 0;
	int iteration = 0;
	while(err != 4) {
		err = fscanf(fp, "%s %d %d %s", colorCode, &COLS, &ROWS, maxColorVal);

		iteration++;
		if(iteration == 10) {
			printf("ERROR: Unable to read file after maximum iterations\nExiting...\n");
			exit(1);
		}	
	}

	printf("Type = %s, ROWS =  %d, COLS = %d, Max Val = %s\n", colorCode, ROWS, COLS, maxColorVal);

	// Determine B/W or RGB

	int bytesPerColor;
	if(strcmp(colorCode, "P5") == 0) {
		bytesPerColor = 1;
	} else if(strcmp(colorCode, "P6") == 0) {
		bytesPerColor = 3;
	} else {
		printf("ERROR: Invalid File Type\nExiting...\n");
		exit(1);
	}

	int totalBytes = ROWS * COLS * bytesPerColor;

	fseek(fp, sizeof(*displaydata), 1); // Jump Junk Data

	// Read in binary data
	unsigned char *inputData = NULL;
	inputData = (unsigned char *) calloc(sizeof(*inputData), totalBytes); 

	err = 0;
	iteration = 0;
	while(err != totalBytes) {
	err = fread(inputData, sizeof(*inputData), totalBytes, fp);
	iteration++;
	if(iteration == 3) {
		printf("Unable to read file after maximum iterations\nExiting...\n");
		exit(1);
		}
	}

	// Convert to 16-bit Color
	int totalBytes16Bit = ROWS*COLS*2;
	displaydata = calloc(sizeof(*displaydata), totalBytes16Bit);

	int i;
	for(i = 0; i < totalBytes16Bit; i+=2) {
		unsigned char redBits = 0x00; 
		unsigned char greenBits = 0x00;
		unsigned char blueBits = 0x00;

		if(bytesPerColor == 1) {
			redBits = inputData[i/2] & 0xF8;
			greenBits = inputData[i/2] & 0xFC; 
			blueBits = redBits;
		} else {
			redBits = inputData[(3*i)/2] & 0xF8;
			greenBits = inputData[(3*i)/2+1] & 0xFC;
			blueBits = inputData[(3*i)/2+2] & 0xF8;
		}

		displaydata[i+1] |= redBits | (greenBits >> 5);
		displaydata[i] |= (greenBits << 3) | (blueBits >> 3); 
	}

	Monitor=XOpenDisplay(NULL);
	if (Monitor == NULL) {
	 	printf("Unable to open graphics display\n");
	 	exit(0);
	 }

	ImageWindow=XCreateSimpleWindow(
		Monitor,
		RootWindow(Monitor,0),
		50,
		10,	 /* x,y on screen */
		COLS,
		ROWS,	/* width, height */
		2, /* border width */
		BlackPixel(Monitor,0),
		WhitePixel(Monitor,0)
	);

	ImageGC=XCreateGC(Monitor,ImageWindow,0,NULL);

	XMapWindow(Monitor,ImageWindow);
	XFlush(Monitor);

	while(1) {
	 	XGetWindowAttributes(Monitor,ImageWindow,&Atts);
	 	if (Atts.map_state == IsViewable) break;
	}

	Picture=XCreateImage(Monitor, DefaultVisual(Monitor,0), DefaultDepth(Monitor,0), ZPixmap, 0, (char *)displaydata, COLS, ROWS, 16, 0);

	XPutImage(
		Monitor,
		ImageWindow,
		ImageGC,
		Picture,
		0,
		0,
		0,
		0,	/* src x,y and dest x,y offsets */
		COLS,
		ROWS
	);	/* size of the image data */

	XFlush(Monitor);
	printf("Press any key to quit...\n");
	getchar();
	XCloseDisplay(Monitor);
	fclose(fp);

return 0;
}