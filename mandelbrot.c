

/*Parallel for loop times:
 * 2-threads: 57.704517
 * 4-threads: 47.206564
 * 8-threads: 46.912026
*/

/*Task version times:
 * 2-threads: 54.579343
 * 4-threads: 53.859114
 * 8-threads: 42.710033 
*/



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <omp.h>

/* Return the current time in seconds, using a double precision number.       */

double When()

{

    struct timeval tp;

    gettimeofday(&tp, NULL);

    return ((double) tp.tv_sec + (double) tp.tv_usec * 1e-6);

}


int main(int argc, char* argv[])
{
  /* Parse the command line arguments. */
  if (argc != 8) {
    printf("Usage:   %s <xmin> <xmax> <ymin> <ymax> <maxiter> <xres> <out.ppm>\n", argv[0]);
    printf("Example: %s 0.27085 0.27100 0.004640 0.004810 1000 8192 pic.ppm\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* The window in the plane. */
  const double xmin = atof(argv[1]);
  const double xmax = atof(argv[2]);
  const double ymin = atof(argv[3]);
  const double ymax = atof(argv[4]);

  /* Maximum number of iterations, at most 65535. */
  const uint16_t maxiter = (unsigned short)atoi(argv[5]);

  /* Image size, width is given, height is computed. */
  const int xres = atoi(argv[6]);
  const int yres = (xres*(ymax-ymin))/(xmax-xmin);

  /* The output file name */
  const char* filename = argv[7];

  /* Open the file and write the header. */
  FILE * fp = fopen(filename,"wb");
  char *comment="# Mandelbrot set";/* comment should start with # */

  /*write ASCII header to the file*/
  fprintf(fp,
          "P6\n# Mandelbrot, xmin=%lf, xmax=%lf, ymin=%lf, ymax=%lf, maxiter=%d\n%d\n%d\n%d\n",
          xmin, xmax, ymin, ymax, maxiter, xres, yres, (maxiter < 256 ? 256 : maxiter));

  /* Precompute pixel width and height. */
  double dx=(xmax-xmin)/xres;
  double dy=(ymax-ymin)/yres;

  double x, y; /* Coordinates of the current point in the complex plane. */
  double u, v; /* Coordinates of the iterated point. */
  int i,j; /* Pixel counters */
  int k; /* Iteration counter */
  
  unsigned char*** pxls = malloc(sizeof(char**)*xres);//[xres][yres][6];
  for(int i = 0; i < xres; i++)
  {
      pxls[i] = malloc(sizeof(char*)*yres);
      for(int j = 0; j < yres; j++){
          pxls[i][j] = malloc(sizeof(char)*6);
      }
  }
  int num_threads = 0;
# ifdef _OPENMP 
	printf("Compiled by an OpenMP-compliant implementation.\n");
    num_threads = 4;
    omp_set_num_threads(num_threads);
    printf("Num threads: %i\n", num_threads);
# endif

double start = When();
#pragma omp parallel 
    { 
        #pragma omp single private(x,y,k,i)
        { 
            for (j = 0; j < yres; j++) {
                y = ymax - j * dy;
                
                
                    #pragma omp task firstprivate(j)
                    { 
                        for(i = 0; i < xres; i++) {
                            double u = 0.0;
                            double v= 0.0;
                            double u2 = u * u;
                            double v2 = v*v;
                            x = xmin + i * dx;
                            /* iterate the point */
                            for (k = 1; k < maxiter && (u2 + v2 < 4.0); k++) {
                                    v = 2 * u * v + y;
                                    u = u2 - v2 + x;
                                    u2 = u * u;
                                    v2 = v * v;
                            }
                            /* compute  pixel color and write it to file */
                            if (k >= maxiter) {
                                /* interior */
                                unsigned char black[] = {0, 0, 0, 0, 0, 0};
                                memcpy(pxls[i][j], black, sizeof(unsigned char)*6);
                                //fwrite (black, 6, 1, fp);//instead write to array
                            }
                            else {
                                /* exterior */
                                unsigned char color[6];
                                color[0] = k >> 8;
                                color[1] = k & 255;
                                color[2] = k >> 8;
                                color[3] = k & 255;
                                color[4] = k >> 8;
                                color[5] = k & 255;
                                memcpy(pxls[i][j], color, sizeof(unsigned char)*6);
                                //fwrite(color, 6, 1, fp);//instead write to array
                            }
                        }
                    }
                }
            } 
        } 
    //write single threaded
  for (j = 0; j < yres; j++) {
    for(i = 0; i < xres; i++) {
        fwrite(pxls[i][j], 6, 1, fp);
    }
  }
  double end = When();
  printf("Start Time: %f \n", start);
  printf("End Time: %f \n", end);
  printf("Total Time: %f \n", (end - start));
  fclose(fp);
  return 0;
}
