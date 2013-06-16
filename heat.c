#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* ~~~~~~~~~ */
/* Constants */
/* ~~~~~~~~~ */

#define DENSITY (8.96e3) /* kg per m^3 */
#define CONDUCTIVITY 401.0 /* W per mK */
#define SPEC_HEAT_CAP 390.0 /* J per kgK */

#define WIDTH 10 /* mm, each point is a millimeter */

#define ITERATIONS 3000000

/* ~~~~~~~~~~~~~~~ */
/* Macro functions */
/* ~~~~~~~~~~~~~~~ */
#define current(i,j) u_current[(i)*WIDTH + (j)]
#define previous(i,j) u_previous[(i)*WIDTH + (j)]

#define inbounds(i,j) ((i) >= 0 && (j) >= 0 && (i) <= WIDTH && (j) <= WIDTH)

/* ~~~~~~~~~~~~~~~~ */
/* Global Variables */
/* ~~~~~~~~~~~~~~~~ */

FILE * output_file = NULL;

double * u_current = NULL;
double * u_previous = NULL;

/* The time interval for recalculation (in seconds) */
const double delta_t = 0.0001;

/* ~~~~~~~~~~~~ */
/* Program Body */
/* ~~~~~~~~~~~~ */

void print_grid(int t)
{
	printf("~~ t = %.6lfs ~~\n", t*delta_t);
	for (int i = 0; i < WIDTH; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			printf("%.1lf ", current(i,j));
		}
		printf("\n");
	}
}

int main()
{
	/* Open the output file for writing */
	output_file = fopen("output.txt", "w");

	if (output_file == NULL)
	{
		perror("output file");
		exit(EXIT_FAILURE);
	}

	/* The heat equation coefficient */
	const double alpha = (CONDUCTIVITY/(DENSITY*SPEC_HEAT_CAP));

	/* The spacing between points in the plane */
	const double delta_s = 0.001;


	double dt_limit = (delta_s*delta_s)/(2*alpha);

	if (delta_t >= dt_limit)
	{
		printf("Please set a time interval less than %.5lf seconds\n", dt_limit);
		exit(EXIT_FAILURE);
	}

	/* Create two arrays of temperature values, current and previous */
	/* Access the (i, j)th entry of each matrix by [i*WIDTH + j] */
	u_current = malloc(WIDTH*WIDTH*sizeof(double));
	u_previous = malloc(WIDTH*WIDTH*sizeof(double));

	if (u_current == NULL || u_previous == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	/* Make the first half of the sheet really hot, and the second cold */
	for (int i = 0; i < WIDTH; i++)
	{
		for (int j = 0; j < WIDTH/2; j++)
		{
			current(i, j) = 550.5;
			previous(i, j) = 550.5;
		}
		for (int j = WIDTH/2; j < WIDTH; j++)
		{
			current(i, j) = 200.2;
			previous(i, j) = 200.2;
		}
	}

	print_grid(0.0);

	/* During each iteration the terms are multiplied by this constant */
	const double coeff = (alpha*delta_t)/(delta_s*delta_s);

	double sum = 0;

	/* Run! */
	int t = 0;
	for (t = 0; t < ITERATIONS; t++)
	{
		/* Make the current temps the new previous ones */
		double * temp = u_previous;
		u_previous = u_current;
		u_current = temp;

		/* Recalculate the temperature at each position */
		for (int i = 1; i < WIDTH - 1; i++)
		{
			for (int j = 1; j < WIDTH - 1; j++)
			{
				sum = 0;

				if (inbounds(i + 1, j))
				{
					sum += previous(i + 1, j);
				}

				if (inbounds(i - 1, j))
				{
					sum  += previous(i - 1, j);
				}

				if (inbounds(i, j + 1))
				{
					sum += previous(i, j + 1);
				}

				if (inbounds(i, j - 1))
				{
					sum += previous(i, j - 1);
				}

				sum -= 4*(previous(i, j));

				current(i, j) = previous(i,j) + coeff*sum;
			}
		}

		// print_grid(t);
	}

	print_grid(t);

	fclose(output_file);

	free(u_current);
	free(u_previous);

	exit(EXIT_SUCCESS);
}
