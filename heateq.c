#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* ~~~~~~~~~ */
/* Constants */
/* ~~~~~~~~~ */

#define DENSITY (8.96e3) /* kg per m^3 */
#define CONDUCTIVITY 401.0 /* W per mK */
#define SPEC_HEAT_CAP 390.0 /* J per kgK */

#define DEFAULT_WIDTH 10 /* mm */
#define DEFAULT_HEIGHT 10 /* mm */

#define ITERATIONS 500

/* ~~~~~~~~~~~~~~~ */
/* Macro functions */
/* ~~~~~~~~~~~~~~~ */

/* Matrix access function */
#define current(i,j) u_current[(i)*width + (j)]
#define previous(i,j) u_previous[(i)*width + (j)]

#define inbounds(i,j) ((i) >= 0 && (j) >= 0 && (i) <= height && (j) <= width)

/* ~~~~~~~~~~~~~~~~ */
/* Global Variables */
/* ~~~~~~~~~~~~~~~~ */

FILE * output_file = NULL;

/* Matrices of temperatures */
/* Access to the (i, j)th entry of each matrix is through the macros above */
double * u_current = NULL;
double * u_previous = NULL;

/* The time interval for recalculation (in seconds) */
double delta_t = 0.0001;

/* The spacing between points in the plane (in metres) */
const double delta_s = 0.001;

/* The size parameters for the material of interest */
int width = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;

/* The number of iterations to perform */
int num_iterations = ITERATIONS;

/* ~~~~~~~~~~~~ */
/* Program Body */
/* ~~~~~~~~~~~~ */

/* Print the temperature grid to the output file */
void output_grid(int t)
{
	fprintf(output_file, "~~ t = %.6lfs ~~\n", t*delta_t);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			fprintf(output_file, "%.1lf ", current(i,j));
		}
		fprintf(output_file, "\n");
	}
}

int main(int argc, char ** argv)
{
	printf("~~~ heateq, by Michael Sproul. 2013 ~~~\n");

	/* Parse the command line arguments */
	int opt;
	int error = 0;
	int retval = 0;
	while ((opt = getopt(argc, argv, "s:I:t:")) != -1)
	{
		switch (opt)
		{
		case 's':
			retval = sscanf(optarg, "%dx%d ", &width, &height);
			if (retval != 2)
			{
				error = 1;
			}
			break;
		case 'I':
			num_iterations = atoi(optarg);
			break;
		case 't':
			retval = sscanf(optarg, "%lf ", &delta_t);
			if (retval != 1)
			{
				error = 1;
			}
			break;
		default:
			break;
		}
	}

	if (error)
	{
		printf("Malformed arguments, quitting.\n");
		exit(EXIT_FAILURE);
	}

	#ifdef _DEBUG
	printf("size: %dmm x %dmm, iterations: %d, delta-t: %lf\n", width, height, num_iterations, delta_t);
	#endif

	/* Calculate the different equation coefficient, alpha */
	const double alpha = (CONDUCTIVITY/(DENSITY*SPEC_HEAT_CAP));

	/* Check that the time interval is small enough */
	double dt_limit = (delta_s*delta_s)/(2*alpha);

	if (delta_t >= dt_limit)
	{
		printf("Please set a time interval less than %.5lf seconds\n", dt_limit);
		exit(EXIT_FAILURE);
	}

	/* Open the output file for writing */
	output_file = fopen("output.txt", "w");

	if (output_file == NULL)
	{
		perror("output file");
		exit(EXIT_FAILURE);
	}

	/* Create two arrays of temperature values, current and previous */
	u_current = malloc(width*height*sizeof(double));
	u_previous = malloc(width*height*sizeof(double));

	if (u_current == NULL || u_previous == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	/* ~~~~~~~~~~~~~~ */
	/* Initial Values */
	/* ~~~~~~~~~~~~~~ */

	/* Make the first half of the sheet really hot, and the second cold */
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width/2; j++)
		{
			current(i, j) = 550.5;
			previous(i, j) = 550.5;
		}
		for (int j = width/2; j < width; j++)
		{
			current(i, j) = 200.2;
			previous(i, j) = 200.2;
		}
	}

	output_grid(0.0);

	/* Calculate the heat equation coefficients */
	const double coeff = (alpha*delta_t)/(delta_s*delta_s);

	double sum = 0;

	printf("Material size: %dmm x %dmm\n", width, height);
	printf("α: %.6lf m²/s, Δt: %.5lf s\n", alpha, delta_t);
	printf("Iterations: %d\n", num_iterations);
	printf("Calculating...");

	/* Run! */
	int t = 0;
	for (t = 0; t < num_iterations; t++)
	{
		/* Make the current temps the previous ones */
		double * temp = u_previous;
		u_previous = u_current;
		u_current = temp;

		/* Recalculate the temperature at each (non edge) position */
		for (int i = 1; i < height - 1; i++)
		{
			for (int j = 1; j < width - 1; j++)
			{
				sum = 0;
				sum += previous(i + 1, j);
				sum  += previous(i - 1, j);
				sum += previous(i, j + 1);
				sum += previous(i, j - 1);
				sum -= 4*(previous(i, j));

				current(i, j) = previous(i,j) + coeff*sum;
			}
		}
	}

	printf(" [done]\n");

	output_grid(t);

	fclose(output_file);
	free(u_current);
	free(u_previous);

	printf("Results written to output.txt\n");

	exit(EXIT_SUCCESS);
}
