/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"
#include <pthread.h>

// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
void* compute_image( void *argv);
static void show_help();

typedef struct{
    imgRawImage* img;
     double xmin;
     double xmax;
     double ymin;
     double ymax;
     int max;
     int width;
     int height;
	 int previous_width;
}compute;

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	const char *outfile = "mandel.jpg";
	double xcenter = 0.3509991;
	double ycenter = 0.09711805;
	double xscale = 0.000001;
	double yscale = 1; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
    int thread = 15;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:h:t:"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help();
				exit(1);
				break;
            case 't':
                thread = atoi(optarg);
                break;
		}
	}

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale = xscale / image_width * image_height;

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf xscale=%.10f yscale=%.10f max=%d outfile=%s\n",xcenter,ycenter,xscale,yscale,max,outfile);

	// Create a raw image of the appropriate size.
	imgRawImage* img = initRawImage(image_width,image_height);

	// Fill it with a black
	setImageCOLOR(img,0);

	// Compute the Mandelbrot image
	//UPDATED CODE

	//sets the argument struct base for the thread calls
    compute argument;
    argument.img = img;
    argument.xmin = xcenter-xscale/2;
    argument.xmax = xcenter+xscale/2;
    argument.ymin = ycenter-yscale/2;
    argument.ymax = ycenter+yscale/2;
    argument.max = max;
    argument.width = 0;
    argument.height = 1000;

	//holds the pthread addresses and arguments for each thread
    pthread_t address[thread];
	compute arguments[thread];
    
	int width = img->width / thread;

	//logic that creates the different threads
	int previous_width = 0;
    for(int i = 1; i <=thread; i++){
		arguments[i-1] = argument;
		
        int get_width = width*i;
        if(i == thread){
            get_width = 1000;
        }
        arguments[i-1].width = get_width;
		arguments[i-1].previous_width = previous_width;
		

       int check = pthread_create(&address[i-1],NULL,compute_image, &arguments[i-1]);
	   if(check != 0){
		perror("Thread creation\n");
		exit(1);
	   }
	   previous_width = get_width;
    }
	for(int i = 0; i < thread; i++){
    	pthread_join(address[i], NULL);
	}
   // printf("all threads completed\n");
    
	// Save the image in the stated file.
	storeJpegImageFile(img,outfile);

	// free the mallocs
	freeRawImage(img);

	return 0;
}




/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void *compute_image(void *arg)
{
	//transfer the structs value into the functions params
	int i,j;
    compute *argument = (compute *)arg;
	int width = argument->width;
	int height = argument->height;
    double xmin = argument-> xmin;
    double xmax = argument-> xmax;
    double ymax = argument-> ymax;
    double ymin = argument-> ymin; 
    int max = argument->max;
    imgRawImage* img = argument->img;


	// For every pixel in the image...

	//iterates over columns 
	for(j=0;j<height;j++) {

		for(i=argument->previous_width;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/1000;
			double y = ymin + j*(ymax-ymin)/1000;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(img,i,j,iteration_to_color(iters,max));
		}
	}
	return NULL;
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}
