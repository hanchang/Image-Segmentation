// #define DEBUG_HISTOGRAM 1
// #define DEBUG_PEAKINESS 1
// #define DEBUG_ALPHA 1

const int INF = 65535;
const int WHITE = 255;
const int BLACK = 0;
const int UNTOUCHED = 42;
const int MAXVAL = 200;
const int MINAREA = 100;

// Loop iterators.
int i, j, k, x;

void sniperBlur(grayscaleimage* image, int xp, int yp, int times); 
void backgroundize(grayscaleimage* image, int histogram_max, int i, int j); 
void gaussianBlur(grayscaleimage* image, int i, int j, int times); 

/* calculatePeakiness------------------------------------------------------*/
/* ------------------------------------------------------------------------*/
// {{{ 
int calculatePeakiness(grayscaleimage* image, int xstart, int xstop) 
{
	// Range of minimum distances to try.
	const int start_min_distance = 2;
	const int stop_min_distance  = 32;
	int used_distance = 0;

	// g1 : Highest histogram count.
	// gi : The gray value where g1 occurs; H(gi) = g1.
	int g1, gi; 

	// g2 : Second highest histogram count at least minimum_distance from g1..
	// gj : The gray value where g2 occurs: H(gj) = g2.
	int g2, gj; 

	// gmin: Smallest histogram value between gi and gj.
	// gk : The gray value where gmin occurs: H(gk) = gmin.
	int gmin, gk; 

	double peakiness = 0;
	double max_peakiness = 0;
	int threshold = 0;
	int sum = 0;
	int light = 0;
	int dark = 0;

	// Initialize histogram.
	unsigned int histogram[256];

	for (i = 0; i < 256; i++) {
		histogram[i] = 0;
	}

	for (i = 0; i < image->ydim; i++) {
		for (j = xstart; j < xstop; j++) {
			histogram[image->value[i][j]] += 1;
		}
	}

	// Grab highest value in the histogram.
	g1 = 0;
	gi = 0;
	for (i = 0; i < MAXVAL; i++) {
		if (histogram[i] > g1) {
			g1 = histogram[i];
			gi = i;
		}
	}

	for (i = 0; i < 256; i++) {
		if (histogram[i] > 10) {
			dark = i;
			break;
		}
	}

	for (i = 255; i > 0; i--) {
		if (histogram[i] > 10) {
			light = i;
			break;
		}
	}

#ifdef DEBUG_PEAKINESS
	printf("Light: %d\n", light);
	printf("Dark: %d\n", dark);
#endif

	if (!((xstart == 0) && (xstop == image->xdim))) {
		// Contrast normalization.
		for (i = 1; i < image->ydim; i++) {
			for (j = xstart + 1; j < xstop - 1; j++) {
				// Reduce black pixels, replacing them with background.
				if (image->value[i][j] <= dark + 18) {
					sniperBlur(image, i, j, 15);
				}

				// Reduce white pixels, replacing them with background.
				if (image->value[i][j] >= light - 18) {
					sniperBlur(image, i, j, 15);
				}
			}
		}

		// Reevaluate histogram after contrast normalization.
		for (i = 0; i < 256; i++) {
			histogram[i] = 0;
		}

		for (i = 0; i < image->ydim; i++) {
			for (j = xstart; j < xstop; j++) {
				if (image->value[i][j] < light - 25) {
					histogram[image->value[i][j]] += 1;
				}
			}
		}
	}

#ifdef DEBUG_HISTOGRAM
		for (i = 0; i < 256; i++) {
			printf("histogram[%d]: %d\n", i, histogram[i]);
		}
#endif

	// Grab highest value in the histogram.
	g1 = 0;
	gi = 0;
	for (i = 0; i < MAXVAL; i++) {
		if (histogram[i] > g1) {
			g1 = histogram[i];
			gi = i;
		}
	}

	for (x = start_min_distance; x < stop_min_distance; x++) {
		// Grab second highest value in the histogram that is at least 
		// minimum_distance away from the highest value.
		g2 = 0;
		gj = 0;
		for (j = 0; j < MAXVAL; j++) {
			if (j > gi - x && j < gi + x) {}
			else if (histogram[j] > g2) {
				g2 = histogram[j];
				gj = j;
			}
		}

		// Find the lowest point gmin in the histogram between g1 and g2.
		gmin = image->ydim * image->xdim;
		if (gi < gj) {
			for (k = gi; k < gj; k++) {
				if (histogram[k] < gmin) {
					gmin = histogram[k];
					gk = k;
				}
			}
		}
		else {
			for (k = gj; k < gi; k++) {
				if (histogram[k] < gmin) {
					gmin = histogram[k];
					gk = k;
				}
			}
		} 

#ifdef DEBUG_PEAKINESS
		printf("Highest value %d found at histogram[%d].\n", g1, gi);
		printf("2nd highest value %d found at histogram[%d].\n", g2, gj);
		printf("Smallest value %d found between at histogram[%d].\n", gmin, gk);
#endif

		// Find the maximum peakiness.
		if (g1 < g2) {
			peakiness = (double) g1 / (double) gmin;
		}
		else {
			peakiness = (double) g2 / (double) gmin;
		}

		if (peakiness > max_peakiness && peakiness != 0 && peakiness < INF) {
			max_peakiness = peakiness;
			threshold = gk;
			used_distance = x;
		}

#ifdef DEBUG_PEAKINESS
		printf("Peakiness: %f\n", peakiness);
#endif
	} // end minimum distance for loop
#ifdef DEBUG_PEAKINESS
	printf("Maximum Peakiness: %f at %d with distance %d\n", 
			max_peakiness, threshold, used_distance);
#endif

	return threshold;
}
// }}}

/* normalizeBackground----------------------------------------------------*/
/* ------------------------------------------------------------------------*/
// {{{
void normalizeBackground(grayscaleimage* binary, int xstart, int xstop) {
	int area = 0;
	int i, j;
	for (i = 0; i < binary->ydim; i++) {
		for (j = xstart; j < xstop; j++) {
			if (binary->value[i][j] == BLACK) {
				area++;
			}
		}
	}

	// Swap foreground and background if there is more foreground than back.
	if (area > (binary->xdim * binary->ydim)/4) {
		printf("Inverting binary image.\n");
		for (i = 0; i < binary->ydim; i++) {
			for (j = xstart; j < xstop; j++) {
				if (binary->value[i][j] == BLACK) {
					binary->value[i][j] = WHITE;
				}
				else {
					binary->value[i][j] = BLACK;
				}
			}
		}
	}

	return;
}
// }}}

/* calculateOrientation----------------------------------------------------*/
/* ------------------------------------------------------------------------*/
// {{{
void calculateOrientation(grayscaleimage *binary, int xstart, int xstop, int area) {
	// Average x and y values.
	double xbar = 0;
	double ybar = 0;
	for (i = 0; i < binary->ydim; i++) {
		for (j = xstart; j < xstop; j++) {
			if (binary->value[i][j] == UNTOUCHED) {
				xbar += j;
				ybar += i;
			}
		}
	}
	xbar = xbar / area;
	ybar = ybar / area;

#ifdef DEBUG_ALPHA
	printf("xbar: %f \n", xbar);
	printf("ybar: %f \n", ybar);
	printf("Area for bars: %d \n", area);

	// Make a cross to mark the centroid.
	for (i = 0; i < binary->ydim; i++) {
		for (j = xstart; j < xstop; j++) {
			if (i == (int) ybar && j == (int) xbar) {
				binary->value[i-2][j] = 128;
				binary->value[i-1][j] = 128;
				binary->value[i+1][j] = 128;
				binary->value[i+2][j] = 128;
				binary->value[i][j] = 128;
				binary->value[i][j-2] = 128;
				binary->value[i][j-1] = 128;
				binary->value[i][j+1] = 128;
				binary->value[i][j+2] = 128;
			}
		}
	}
#endif

	// Calculate the a, b/2, and c values.
	double a, b, c;
	for (i = 0; i < binary->ydim; i++) {
		for (j = xstart; j < xstop; j++) {
			if (binary->value[i][j] == UNTOUCHED) {
				a += pow(j - xbar, 2);
				b += (j - xbar) * (i - ybar);
				c += pow(i - ybar, 2);
			}
		}
	}
	b = b * 2;

	// Calculate the angle.
	double theta = 0;
	if (a - c == 0) {
		printf("Divide by zero error.\n");
	}
	else {
		theta = 0.5 * atan(b/(c - a));
	}
	// Convert to degrees.
	theta *= 180/M_PI;

#ifdef DEBUG_ALPHA
	printf("a: %f\n", a);
	printf("b: %f\n", b);
	printf("c: %f\n", c);
#endif

	printf("Orientation: %f degrees\n", theta); 

	double root = sqrt(pow(b,2) + pow(a-c,2));
	double emin = (a+c)/2 - (a-c)/2 * ((a-c)/root) - b/2*(b/root);
	double emax = (a+c)/2 + (a-c)/2 * ((a-c)/root) + b/2*(b/root);

#ifdef DEBUG_ALPHA
	printf("emin: %f\n", emin);
	printf("emax: %f\n", emax);
#endif

	double circularity = emin/emax;

	printf("Circularity: %f\n", circularity);

	return;
}
// }}}

/* recursiveTouch----------------------------------------------------------*/
/* ------------------------------------------------------------------------*/
// {{{
int recursiveTouch(grayscaleimage *binary, rgbimage *output, 
		int i, int j, int color, unsigned long area) 
{
	binary->value[i][j] = BLACK;
	area++;

	// Fun colors!
	if (color % 7 == 0) {
		output->r[i][j] = 255;
		output->g[i][j] = 255;
		output->b[i][j] = 255;
	}
	else if (color % 7 == 1) {
		output->r[i][j] = 0;
		output->g[i][j] = 255;
		output->b[i][j] = 0;
	}
	else if (color % 7 == 2) {
		output->r[i][j] = 0;
		output->g[i][j] = 0;
		output->b[i][j] = 255;
	}
	else if (color % 7 == 3) {
		output->r[i][j] = 255;
		output->g[i][j] = 255;
		output->b[i][j] = 0;
	}
	else if (color % 7 == 4) {
		output->r[i][j] = 255;
		output->g[i][j] = 0;
		output->b[i][j] = 255;
	}
	else if (color % 7 == 5) {
		output->r[i][j] = 0;
		output->g[i][j] = 255;
		output->b[i][j] = 255;
	}
	else {
		output->r[i][j] = 255;
		output->g[i][j] = 0;
		output->b[i][j] = 0;
	}

	if (j - 1 >= 0 && binary->value[i][j-1] == UNTOUCHED) {
		area = recursiveTouch(binary, output, i, j-1, color, area);
	}
	if (j + 1 < binary->xdim && binary->value[i][j+1] == UNTOUCHED) {
		area = recursiveTouch(binary, output, i, j+1, color, area);
	}

	if (i - 1 >= 0) {
		if (j - 1 >= 0 && binary->value[i-1][j-1] == UNTOUCHED) {
			area = recursiveTouch(binary, output, i-1, j-1, color, area);
		}
		if (binary->value[i-1][j] == UNTOUCHED) {
			area = recursiveTouch(binary, output, i-1, j, color, area);
		}
		if (j + 1 < binary->xdim && binary->value[i-1][j+1] == UNTOUCHED) {
			area = recursiveTouch(binary, output, i-1, j+1, color, area);
		}
	}

	if (i + 1 < binary->ydim) {
		if (j - 1 >= 0 && binary->value[i+1][j-1] == UNTOUCHED) {
			area = recursiveTouch(binary, output, i+1, j-1, color, area);
		}
		if (binary->value[i+1][j] == UNTOUCHED) {
			area = recursiveTouch(binary, output, i+1, j, color, area);
		}
		if (j + 1 < binary->xdim && binary->value[i+1][j+1] == UNTOUCHED) {
			area = recursiveTouch(binary, output, i+1, j+1, color, area);
		}
	}

	return area;
}
//}}}

/* BinaryTouch----------------------------------------------------*/
/* ------------------------------------------------------------------------*/
// {{{
void binaryTouch(grayscaleimage *binary, int i, int j) {
	binary->value[i][j] = UNTOUCHED;

	if (j - 1 >= 0 && binary->value[i][j-1] == BLACK) {
		binaryTouch(binary, i, j-1);
	}
	if (j + 1 < binary->xdim && binary->value[i][j+1] == BLACK) {
		binaryTouch(binary, i, j+1);
	}

	if (i - 1 >= 0) {
		if (j - 1 >= 0 && binary->value[i-1][j-1] == BLACK) {
			binaryTouch(binary, i-1, j-1);
		}
		if (binary->value[i-1][j] == BLACK) {
			binaryTouch(binary, i-1, j);
		}
		if (j + 1 < binary->xdim && binary->value[i-1][j+1] == BLACK) {
			binaryTouch(binary, i-1, j+1);
		}
	}

	if (i + 1 < binary->ydim) {
		if (j - 1 >= 0 && binary->value[i+1][j-1] == BLACK) {
			binaryTouch(binary, i+1, j-1);
		}
		if (binary->value[i+1][j] == BLACK) {
			binaryTouch(binary, i+1, j);
		}
		if (j + 1 < binary->xdim && binary->value[i+1][j+1] == BLACK) {
			binaryTouch(binary, i+1, j+1);
		}
	}

	return;
}
//}}}

/* removeSpecks------------------------------------------------------------*/
/* ------------------------------------------------------------------------*/
//{{{
void removeSpecks(grayscaleimage *binary, int i, int j) {
	binary->value[i][j] = WHITE;

	if (j - 1 >= 0 && binary->value[i][j-1] == BLACK) {
		removeSpecks(binary, i, j-1);
	}
	if (j + 1 < binary->xdim && binary->value[i][j+1] == BLACK) {
		removeSpecks(binary, i, j+1);
	}

	if (i - 1 >= 0) {
		if (j - 1 >= 0 && binary->value[i-1][j-1] == BLACK) {
			removeSpecks(binary, i-1, j-1);
		}
		if (binary->value[i-1][j] == BLACK) {
			removeSpecks(binary, i-1, j);
		}
		if (j + 1 < binary->xdim && binary->value[i-1][j+1] == BLACK) {
			removeSpecks(binary, i-1, j+1);
		}
	}

	if (i + 1 < binary->ydim) {
		if (j - 1 >= 0 && binary->value[i+1][j-1] == BLACK) {
			removeSpecks(binary, i+1, j-1);
		}
		if (binary->value[i+1][j] == BLACK) {
			removeSpecks(binary, i+1, j);
		}
		if (j + 1 < binary->xdim && binary->value[i+1][j+1] == BLACK) {
			removeSpecks(binary, i+1, j+1);
		}
	}

	return;
}
//}}}

/* findArea ---------------------------------------------------------------*/
/* ------------------------------------------------------------------------*/
//{{{
int findArea(grayscaleimage* main, grayscaleimage* binary, int a, int z) {
	// Hack to make sure entire single image is counted in area.
	int tmp = 0;
	for (i = 0; i < main->ydim; i++) {
		for (j = a; j < z; j++) {
			if (binary->value[i][j] == BLACK) {
				main->value[i][j] = UNTOUCHED;
				tmp++;
			}
			else {
				main->value[i][j] = WHITE;
			}
		}
	}

	printf("Calculated Area: %f um^2\n", tmp * 0.667223);

	return tmp;
}
//}}}

/* ----- Blur Image for Reduction of Background Noise -----
 * --------------------------------------------------------
 */

//{{{
void backgroundize(grayscaleimage* image, int histogram_max, int xp, int yp) {
	image->value[xp][yp] = 
		(image->value[xp-1][yp-1] + 
		 image->value[xp-1][yp] + 
		 image->value[xp][yp-1]) / 3; 
}
//}}}

//{{{
void sniperBlur(grayscaleimage* image, int xp, int yp, int times) {
	int iter = 0;
	for (iter = 0; iter < times; iter++) {
		// Accentuates white while reducing black.
		image->value[xp][yp] = 
			(10 * image->value[xp-1][yp-1] + 
			 5 * image->value[xp-1][yp] + 
			 5 * image->value[xp][yp-1]) / 18; 
		/*
		   (image->value[xp-1][yp-1] + 
		   image->value[xp-1][yp] + 
		   image->value[xp-1][yp+1] + 

		   image->value[xp][yp-1] + 
		   image->value[xp][yp+1] + 

		   image->value[xp+1][yp-1] + 
		   image->value[xp+1][yp] + 
		   image->value[xp+1][yp+1]) / 8; 
	   */
	}

	return;
}
//}}}

//{{{
void simpleBlur(grayscaleimage image, int times) {
	int i, j, k;
	for (k = 0; k < times; k++) {
		for (i = 1; i < image.ydim - 1; i++) {
			for (j = 1; j < image.xdim - 1; j++) {
				int tmp = 
					image.value[i-1][j-1] + 
					image.value[i-1][j] + 
					image.value[i-1][j+1] + 

					image.value[i][j-1] + 
					2 * image.value[i][j] + 
					image.value[i][j+1] + 

					image.value[i+1][j-1] + 
					image.value[i+1][j] + 
					image.value[i+1][j+1]; 

				image.value[i][j] = tmp / 10;
			}
		}
	}
	image.highestvalue = WHITE;
}
//}}}

//{{{
void mediumBlur(grayscaleimage image, int times) {
	int i, j, k;
	for (k = 0; k < times; k++) {
		for (i = 1; i < image.ydim - 1; i++) {
			for (j = 1; j < image.xdim - 1; j++) {
				int tmp = 
					image.value[i-1][j-1] + 
					2 * image.value[i-1][j] + 
					image.value[i-1][j+1] + 

					2 * image.value[i][j-1] + 
					6 * image.value[i][j] + 
					2 * image.value[i][j+1] + 

					image.value[i+1][j-1] + 
					2 * image.value[i+1][j] + 
					image.value[i+1][j+1]; 

				image.value[i][j] = tmp / 18;
			}
		}
	}
	image.highestvalue = WHITE;
}
//}}}

//{{{
void gaussianBlur(grayscaleimage* image, int i, int j, int times) {
	if (i < 3 || j < 3 || i > image->ydim - 3 || j > image->xdim - 3) {}
	else {
		for (k = 0; k < times; k++) {
			int tmp = 
				2 * image->value[i-2][j-2] + 
				4 * image->value[i-2][j-1] + 
				5 * image->value[i-2][j] + 
				4 * image->value[i-2][j+1] + 
				2 * image->value[i-2][j+2] + 

				4 * image->value[i-1][j-2] +
				9 * image->value[i-1][j-1] + 
				12 * image->value[i-1][j] + 
				9 * image->value[i-1][j+1] + 
				4 * image->value[i-1][j+2] +

				9 * image->value[i][j-2] + 
				12 * image->value[i][j-1] + 
				15 * image->value[i][j] + 
				12 * image->value[i][j+1] + 
				9 * image->value[i][j+2] + 

				4 * image->value[i+1][j-2] +
				9 * image->value[i+1][j-1] + 
				12 * image->value[i+1][j] + 
				9 * image->value[i+1][j+1] + 
				4 * image->value[i+1][j+2] +

				2 * image->value[i+2][j-2] + 
				4 * image->value[i+2][j-1] + 
				5 * image->value[i+2][j] + 
				4 * image->value[i+2][j+1] + 
				2 * image->value[i+2][j+2]; 

			image->value[i][j] = tmp / 115;
		}
	}
}
//}}}


