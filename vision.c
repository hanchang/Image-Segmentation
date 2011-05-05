/**********************************************************************/
/*      Margrit Betke, Image and Video Computing, Spring 2006         */
/**********************************************************************/


#include "vision.h"

/**********************************************************************/
/*   			OUTPUT FUNCTIONS               	      */
/**********************************************************************/

// Output a gray-scale image:
void OutputPgm(grayscaleimage* image)
{
  int x, y;
  FILE *ofp;  /* ofp = output file pointer  */
  
  ofp = fopen(image->name, "w"); 
  
  /* pnm label for grayscale ascii images: pgm */
  fprintf(ofp, "P2\n");  
  fprintf(ofp, "# %s\n", image->name);
  printf("Output of %s  xdim: %d ydim: %d\n", image->name, 
	 image->xdim, image->ydim);
  fprintf(ofp, "%d %d\n", image->xdim, image->ydim);
  fprintf(ofp, "%d\n", image->highestvalue);
  
  for(y = 0; y < image->ydim; y++) {
    for(x= 0; x < image->xdim; x++) 
      fprintf(ofp,"%d ", image->value[y][x]);
    fprintf(ofp,"\n");
  }  
  fclose(ofp);  
}

// Output a color image:
void OutputPpm(rgbimage* image)
{
  int x, y;
  FILE *ofp;  /* ofp = output file pointer  */
    
  ofp = fopen(image->name, "w"); 
  
  //* pnm label for rgb ascii images: pgm */
  fprintf(ofp, "P3\n");  
  fprintf(ofp, "# %s\n", image->name);
  printf("Output of %s  xdim: %d ydim: %d\n", image->name, 
	 image->xdim, image->ydim);
  fprintf(ofp, "%d %d\n", image->xdim, image->ydim);
  fprintf(ofp, "%d\n", image->highestvalue);
  
  for(y = 0; y < image->ydim; y++) {
    for(x= 0; x < image->xdim; x++) {
      fprintf(ofp,"%d ", image->r[y][x]);
      fprintf(ofp,"%d ", image->g[y][x]);
      fprintf(ofp,"%d ", image->b[y][x]);
    }  
    fprintf(ofp,"\n");
  }  
  fclose(ofp);  
}


/**********************************************************************/
/*   			SCANNING FUNCTIONS                         */
/**********************************************************************/

// GetNextSignificantNumber scans in next number and skips white space
// and comments
double GetNextSignificantNumber(FILE *fp)
{
  int ch;
  double num;

  while ((ch = fgetc(fp)) == '\t' || (ch == ' ')
	 || (ch == '\n') || (ch == '#')) {
    if (ch == '#') {
      while ((ch = fgetc(fp)) != '\n')
	;
    }
  }

  ungetc(ch, fp);
  fscanf(fp, "%lf", &num);
  
  return (num);
}

// Input a gray-scale image:
void ScanPgm(grayscaleimage *image)
{
  FILE *inputfp;
  int x, y;
  char ch;
  char buff[300];

  inputfp = fopen(image->name, "rb");
  if(!inputfp) {
    printf("Could not open file %s\n",image->name);
    exit(1);
  }

  fscanf(inputfp, "%s\n", buff);
  if ((strcmp(buff, "P2") != 0) && (strcmp(buff, "P5") != 0)) {  
    printf("File %s does not start with header P2 or P5\n", image->name);
    printf("Not a PGM file!\n");
  }
  
  /* Read the dimensions and the highest pixel value of the image: */
  image->xdim =  (int) GetNextSignificantNumber(inputfp);
  image->ydim =  (int) GetNextSignificantNumber(inputfp);
  image->highestvalue =  (int) GetNextSignificantNumber(inputfp);

  // Allocate memory space for pixel values:
  GetImagePgm(image);

  printf("Scan of %s, xdim = %d, ydim = %d highest = %d\n",
	 image->name, image->xdim, image->ydim,
	 image->highestvalue);

  /* Read the pixel values: */
  if (strcmp(buff, "P2") == 0) {
    for (y = 0; y < image->ydim; y++)
      for(x = 0; x < image->xdim; x++)
        image->value[y][x] = (unsigned char) GetNextSignificantNumber(inputfp);
  }
  else if (strcmp(buff, "P5") == 0) {
    if ((ch = fgetc(inputfp)) != '\n')  
      ungetc(ch, inputfp);
    for (y = 0; y < image->ydim; y++)
      for(x = 0; x < image->xdim; x++)
        image->value[y][x] = (unsigned char) fgetc(inputfp);
  }

  fclose(inputfp);
}

// Input a color image:
void ScanPpm(rgbimage *image)
{
  FILE *inputfp;
  int x, y;
  char ch;
  char buff[300];

  inputfp = fopen(image->name, "rb");
  if(!inputfp) {
    printf("Could not open file %s\n",image->name);
    exit(1);
  }

  fscanf(inputfp, "%s\n", buff);
  if ((strcmp(buff, "P3") != 0) && (strcmp(buff, "P6") != 0)) {  
    printf("File %s does not start with header P3 or P6\n", image->name);
    printf("Not a Ppm file!\n");
  }
  
  /* Read the dimensions and the highest pixel value of the image: */
  image->xdim =  (int) GetNextSignificantNumber(inputfp);
  image->ydim =  (int) GetNextSignificantNumber(inputfp);
  image->highestvalue =  (int) GetNextSignificantNumber(inputfp);

  // Allocate memory space for color pixel values:
  GetImagePpm(image);

  printf("Scan of %s, xdim = %d, ydim = %d highest = %d\n",
	 image->name, image->xdim, image->ydim,
	 image->highestvalue);

  /* Read the pixel values: */
  if (strcmp(buff, "P3") == 0) {
    for (y = 0; y < image->ydim; y++)
      for(x = 0; x < image->xdim; x++) {
        image->r[y][x] = (unsigned char) GetNextSignificantNumber(inputfp);
        image->g[y][x] = (unsigned char) GetNextSignificantNumber(inputfp);
        image->b[y][x] = (unsigned char) GetNextSignificantNumber(inputfp);
      }
    fscanf(inputfp, "\n");    
  }
  else if (strcmp(buff, "P6") == 0) {
    if ((ch = fgetc(inputfp)) != '\n')  
      ungetc(ch, inputfp);
    for (y = 0; y < image->ydim; y++)
      for(x = 0; x < image->xdim; x++) {
        image->r[y][x] = (unsigned char) fgetc(inputfp);
        image->g[y][x] = (unsigned char) fgetc(inputfp);
        image->b[y][x] = (unsigned char) fgetc(inputfp);
      }  
  }

  fclose(inputfp);
}

/**********************************************************************/
/*   		MEMORY ALLOCATION FUNCTIONS                         */
/**********************************************************************/

// Allocate memory space for gray-scale pixel values:
void GetImagePgm(grayscaleimage *image)
{
  int x, y;

  image->value = (unsigned char **) malloc(image->ydim*sizeof(unsigned char *));
    if(!image->value) {
	printf("Can't allocate column of image pointers\n");
    	exit(1);
    }
  for(y=0; y<image->ydim; y++){
    image->value[y] = (unsigned char *) malloc(image->xdim*sizeof(unsigned char ));
    if(!image->value[y]) {
	printf("Can't allocate rows of gray-scale pixels\n");
    	exit(1);
    }
  }
}

// Allocate memory space for color pixel values:
void GetImagePpm(rgbimage *image)
{
  int x, y;

  image->r = (unsigned char **) malloc(image->ydim*sizeof(unsigned char *));
    if(!image->r) {
	printf("Can't allocate column of red image pointers\n");
    	exit(1);
    }
  image->g = (unsigned char **) malloc(image->ydim*sizeof(unsigned char *));
    if(!image->g) {
	printf("Can't allocate column of green image pointers\n");
    	exit(1);
    }
  image->b = (unsigned char **) malloc(image->ydim*sizeof(unsigned char *));
    if(!image->b) {
	printf("Can't allocate column of blue image pointers\n");
    	exit(1);
    }
  for(y=0; y<image->ydim; y++){
    image->r[y] = (unsigned char *) malloc(image->xdim*sizeof(unsigned char ));
    if(!image->r[y]) {
	printf("Can't allocate red image rows\n");
    	exit(1);
    }
  }
  for(y=0; y<image->ydim; y++){
    image->g[y] = (unsigned char *) malloc(image->xdim*sizeof(unsigned char ));
    if(!image->g[y]) {
	printf("Can't allocate green image rows\n");
    	exit(1);
    }
  }
  for(y=0; y<image->ydim; y++){
    image->b[y] = (unsigned char *) malloc(image->xdim*sizeof(unsigned char ));
    if(!image->b[y]) {
	printf("Can't allocate blue image rows\n");
    	exit(1);
    }
  }
}
