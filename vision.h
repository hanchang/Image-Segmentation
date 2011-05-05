/**********************************************************************/
/*      Margrit Betke, Image and Video Computing, Spring 2006         */
/**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXNAME 100

typedef struct {
  char name[MAXNAME];
  int xdim;
  int ydim;
  int highestvalue; 
  unsigned char **value; 
} grayscaleimage;

typedef struct {
  char name[MAXNAME];
  int xdim;
  int ydim;
  int highestvalue; 
  unsigned char **r; 
  unsigned char **g; 
  unsigned char **b; 
} rgbimage;


void OutputPgm(grayscaleimage *);
void OutputPpm(rgbimage *);

void ScanPgm(grayscaleimage *);
void ScanPpm(rgbimage *);

void GetImagePgm(grayscaleimage *);
void GetImagePpm(rgbimage *);


