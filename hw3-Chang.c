// Written by Margrit Betie for Image and Video Computing, January 2006
// Compile with 
//   g++ -o hw1-YourLastName  main.c vision.c -lm
// Run with:
//   ./hw1-YourLastName input-image.ppm

#include <math.h>
#include "vision.h"
#include "helpers.c"

// #define DEBUG_HISTOGRAM 1
// #define DEBUG_BINARY 1
// #define DEBUG_ALPHA 1
// #define DEBUG_IMAGE 1

int main(int argc, char *argv[]) {
	bool use_peakiness = true;
	int num_images = 1;

	if (argc != 2) {
		printf("Usage: ./hw3-Chang inputimage.ppm \n");
		exit(1);
	}
	/*
	if (argc == 3 && strcmp(argv[2], "iterative") == 0) {
		use_peakiness = false;
	}
	*/

	// Loop iterators.
	int i, j, k, x, y, z; 
	int threshold = 0;

	// Grab image, place in memory.
	grayscaleimage image;
	grayscaleimage binary;
	grayscaleimage mainobject;
	char imagename[30];
	strcpy(image.name, argv[1]);
	strcpy(binary.name, argv[1]);
	strcpy(mainobject.name, argv[1]);
	ScanPgm(&image);
	ScanPgm(&binary);
	ScanPgm(&mainobject);

	if (image.xdim > 275)
		num_images = 2;

	/* ----- Peakiness Detection for Appropriate Threshold Selection ----- 
	 * -------------------------------------------------------------------
	 */

	if (use_peakiness && num_images == 1) {
		threshold = calculatePeakiness(&image, 0, image.xdim);
		// printf("Threshold: %d\n", threshold);

		// Create a binary output image based on the threshold created.
		for (i = 0; i < binary.ydim; i++) {
			for (j = 0; j < binary.xdim; j++) {
				if ((unsigned int) image.value[i][j] >= threshold) {
					binary.value[i][j] = WHITE;
				}
				else {
					binary.value[i][j] = BLACK;
				}
			}
		}

		normalizeBackground(&binary, 0, binary.xdim);

	} // PEAKINESS , num_images 1

	else if (use_peakiness && num_images == 2) {
		int threshold1 = calculatePeakiness(&image, 0, image.xdim/2);
		int threshold2 = calculatePeakiness(&image, image.xdim/2, image.xdim);

		//printf("Threshold: %d\n", threshold1);

		mediumBlur(image, 1);

#ifdef DEBUG_IMAGE
		strcpy(image.name, "blur.pgm");
		OutputPgm(&image);
#endif

		image.highestvalue = WHITE;
		binary.highestvalue = WHITE;

		// Create a binary output image based on the threshold created.
		for (i = 0; i < binary.ydim; i++) {
			for (j = 0; j < binary.xdim; j++) {
				if (j < binary.xdim / 2) {
					if ((unsigned int) image.value[i][j] > threshold1) {
						binary.value[i][j] = WHITE;
					}
					else {
						binary.value[i][j] = BLACK;
					}
				}
				else {
					if ((unsigned int) image.value[i][j] > threshold1) {
						binary.value[i][j] = WHITE;
					}
					else {
						binary.value[i][j] = BLACK;
					}
				}
			}
		}

		normalizeBackground(&binary, 0, binary.xdim/2);
		normalizeBackground(&binary, binary.xdim/2, binary.xdim);

#ifdef DEBUG_IMAGE
		strcpy(binary.name, "preprocess.pgm");
		OutputPgm(&binary);
#endif

	} // PEAKINESS, num_images 2

	//{{{
	else {
		/* ----- Iterative Threshold Selection -----
		 * -----------------------------------------
		 */
		double mean1, mean2;
		bool notequal = true;

		// Find the average intensity of the image.
		threshold = 0;
		for (i = 0; i < image.ydim; i++) {
			for (j = 0; j < image.xdim; j++) {
				threshold += image.value[i][j];
			}
		}
		threshold /= (image.ydim * image.xdim);

		while (notequal) {
			double tmp1, tmp2;
			// Partition image into two groups.
			for (i = 0; i < image.ydim; i++) {
				for (j = 0; j < image.xdim; j++) {
					if (image.value[i][j] > threshold)
						tmp1 += image.value[i][j];
					else
						tmp2 += image.value[i][j];
				}
			}
			tmp1 /= (image.ydim * image.xdim);
			tmp2 /= (image.ydim * image.xdim);

			if (tmp1 == mean1 && tmp2 == mean2) {
				// Threshold found; break out of loop.
				notequal = false;
			}
			else {
				// Select a new threshold.
				mean1 = tmp1;
				mean2 = tmp2;
			}
			threshold = (int) (mean1 + mean2) / 2;
		}

		// Make image binary. 
		for (i = 0; i < binary.ydim; i++) {
			for (j = 0; j < binary.xdim; j++) {
				if (binary.value[i][j] > threshold) {
					binary.value[i][j] = WHITE;
				}
				else {
					binary.value[i][j] = BLACK;
				}
			}
		}
	} // ITERATIVE
	//}}}

	/* ----- Connected Component -----
	 * -------------------------------
	 */

	bool stillUntouched = true;

	// Initialize by negation.
	for (i = 0; i < binary.ydim; i++) {
		for (j = 0; j < binary.xdim; j++) {
			if (binary.value[i][j] == BLACK) {
				binary.value[i][j] = UNTOUCHED;
			}
		}
	}

	// Prepare to count objects.
	int object1 = 0;
	int object2 = 0;
	int remove1 = 0;
	int remove2 = 0;

	// Create a color copy.
	int color = 1;
	rgbimage colorimage;
	colorimage.xdim = image.xdim;
	colorimage.ydim = image.ydim;
	colorimage.highestvalue = WHITE;

	GetImagePpm(&colorimage);
	for (i = 0; i < colorimage.ydim; i++) {
		for (j = 0; j < colorimage.xdim; j++) {
			colorimage.r[i][j] = 255;
			colorimage.g[i][j] = 255;
			colorimage.b[i][j] = 255;
		}
	}

	// Find the first untouched pixel.
	unsigned long tmparea = 0;
	unsigned long maxarea = 0;
	int maxi, maxj;
	int maxx, maxy;

	if (num_images == 1) {
		for (i = 0; i < colorimage.ydim; i++) {
			for (j = 0; j < colorimage.xdim; j++) {
				if (binary.value[i][j] == UNTOUCHED) {
					// Recurse around that pixel's neighbors.
					tmparea = recursiveTouch(&binary, &colorimage, i, j, color, 0);
					//printf("Segment has area %d with color %d.\n", tmparea, color % 6); //flag

					object1++;

					if (tmparea < MINAREA && tmparea > 0) {
						recursiveTouch(&binary, &colorimage, i, j, 0, 0);
						removeSpecks(&binary, i, j);
						remove1++;
					}

					if (tmparea > maxarea) {
						maxarea = tmparea;
						maxi = i;
						maxj = j;
					}

					color++;
				}
			}
		}
	}
	else {
		// Run through first half of image.
		for (i = 0; i < colorimage.ydim; i++) {
			for (j = 0; j < colorimage.xdim/2; j++) {
				if (binary.value[i][j] == UNTOUCHED) {
					// Recurse around that pixel's neighbors.
					tmparea = recursiveTouch(&binary, &colorimage, i, j, color, 0);
					//printf("Segment has area %d with color %d.\n", tmparea, color % 6); //flag

					object1++;

					if (tmparea < MINAREA && tmparea > 0) {
						recursiveTouch(&binary, &colorimage, i, j, 0, 0);
						removeSpecks(&binary, i, j);
						remove1++;
					}

					if (tmparea > maxarea) {
						maxarea = tmparea;
						maxi = i;
						maxj = j;
					}

					color++;
				}
			}
		}

		// Run through second half of image.
		for (i = 0; i < colorimage.ydim; i++) {
			for (j = colorimage.xdim/2; j < colorimage.xdim; j++) {
				if (binary.value[i][j] == UNTOUCHED) {
					// Recurse around that pixel's neighbors.
					tmparea = recursiveTouch(&binary, &colorimage, i, j, color, 0);
					//printf("Segment has area %d with color %d.\n", tmparea, color % 6);

					object2++;

					if (tmparea < MINAREA && tmparea > 0) {
						recursiveTouch(&binary, &colorimage, i, j, 0, 0);
						removeSpecks(&binary, i, j);
						remove2++;
					}

					if (tmparea > maxarea) {
						maxarea = tmparea;
						maxx = i;
						maxy = j;
					}

					color++;
				}
			}
		}
	}

	strcpy(colorimage.name, "connected.ppm");
	OutputPpm(&colorimage);

	binary.highestvalue = WHITE;
	strcpy(binary.name, "binary.pgm");
	OutputPgm(&binary);

	/* ----- Orientation ----- 
	 * -----------------------
	 */

	printf("\n");

	// Only calculate orientation and circularity for largest blob.
	// Find largest blob and mark it specially.
	if (num_images == 1) {
		int singlearea = findArea(&mainobject, &binary, 0, binary.xdim);
		calculateOrientation(&mainobject, 0, mainobject.xdim, singlearea);
		printf("Components detected: %d\n", object1);
	}
	else {
		int area1 = findArea(&mainobject, &binary, 0, binary.xdim/2);
		binaryTouch(&mainobject, maxi, maxj);
		calculateOrientation(&mainobject, 0, binary.xdim/2, area1);
		// printf("Components detected: %d\n", object1 - remove1);

		printf("\n");

		int area2 = findArea(&mainobject, &binary, binary.xdim/2, binary.xdim);
		binaryTouch(&mainobject, maxx, maxy);
		calculateOrientation(&mainobject, binary.xdim/2, binary.xdim, area2);
		// printf("Components detected: %d\n", object2 - remove2);

		printf("\nTotal components detected: %d\n", object1 + object2);
	}

	/* ------ Region Merging ------
	 * ----------------------------
	 */


	printf("\n");

	/* ----- */
	return 0;
}
