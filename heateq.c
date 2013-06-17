#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

/* ~~~~~~~~~ */
/* Constants */
/* ~~~~~~~~~ */

#define DENSITY (8.96e3) /* kg per m^3 */
#define CONDUCTIVITY 401.0 /* W per mK */
#define SPEC_HEAT_CAP 390.0 /* J per kgK */

#define DEFAULT_WIDTH 20 /* mm */
#define DEFAULT_HEIGHT 20 /* mm */

#define ITERATIONS 500

/* ~~~~~~~~~~~~~~~ */
/* Macro functions */
/* ~~~~~~~~~~~~~~~ */

/* Matrix access function */
#define current(i,j) u_current[(i)*width + (j)]
#define previous(i,j) u_previous[(i)*width + (j)]

/* Functions to determine if the second derivatives exist */
#define has_x_deriv(i,j) ((j + 1) < width && (j - 1) >= 0)
#define has_y_deriv(i,j) ((i + 1) < height && (i - 1) >= 0)

/* ~~~~~~~~~~~~~~~~ */
/* Global Variables */
/* ~~~~~~~~~~~~~~~~ */

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

/* The coefficient used in each computation */
double coeff;

/* A barrier to synchronise threads after each iteration */
pthread_barrier_t iteration_barrier;

/* ~~~~~~~ */
/* Structs */
/* ~~~~~~~ */

/* A struct for dividing the work evenly between threads */
struct work_chunk
{
	int start;
	int end;
	int id;
};

/* ~~~~~~~~~~~~ */
/* Program Body */
/* ~~~~~~~~~~~~ */

/* Safely quit the program */
void quit(int status)
{
	free(u_current);
	free(u_previous);

	exit(status);
}

/* Print the temperature grid to file (gnuplot compatible) */
void output_grid(char * filename)
{
	FILE * output_file = fopen(filename, "w");

	if (output_file == NULL)
	{
		perror("output file");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < height; i++)
	{
		fprintf(output_file, "%.2lf", current(i,0));
		for (int j = 1; j < width; j++)
		{
			fprintf(output_file, " %.2lf", current(i,j));
		}
		fprintf(output_file, "\n");
	}

	fclose(output_file);
}

/* A function to parse commandline arguments */
void parse_args(int argc, char ** argv)
{
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
	printf("size: %dmm x %dmm, iterations: %d, delta-t: %lf\n",
				width, height, num_iterations, delta_t);
	#endif
}

/* Set up the sheet with some starting temperatures */
void assign_initial_temps()
{
	/* Example: make the first half of the sheet really hot, and the second cold */
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width/2; j++)
		{
			previous(i, j) = 550.5;
		}
		for (int j = width/2; j < width; j++)
		{
			previous(i, j) = 200.2;
		}
	}

	/* Make the corners room temp? */
	previous(0,0) = 300;
	previous(0, width - 1) = 300;
	previous(height - 1, 0) = 300;
	previous(height - 1, width - 1) = 300;
}

#ifdef _SEQUENTIAL
/* Calculate the system's evolution sequentially */
void sequential_calc()
{
	/* Run! */
	int t = 0;
	double sum = 0;
	for (t = 0; t < num_iterations; t++)
	{
		/* Recalculate the temperature at each position */
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				sum = 0;

				/* Add the second y derivative, if it exists */
				if (has_y_deriv(i,j))
				{
					sum +=  previous(i + 1, j) +
						previous(i - 1, j) -
						2*previous(i,j);
				}

				/* Add the second x derivative, if it exists */
				if (has_x_deriv(i,j))
				{
					sum +=  previous(i, j + 1) +
						previous(i, j - 1) -
						2*previous(i, j);
				}

				current(i, j) = previous(i,j) + coeff*sum;
			}
		}

		/* Make the current temps the previous ones */
		double * temp = u_previous;
		u_previous = u_current;
		u_current = temp;
	}

	/* After the final run, keep the temps */
	double * temp = u_previous;
	u_previous = u_current;
	u_current = temp;
}
#endif

#ifndef _SEQUENTIAL
/* The thread function for parallel computation */
void *thread_calc(void * v_work)
{
	struct work_chunk * work = (struct work_chunk *) v_work;
	int start = work->start;
	int end = work->end;
	int id = work->id;

	int sum = 0;

	for (int t = 0; t < num_iterations; t++)
	{
		/* Calculate */
		for (int i = start; i < end; i++)
		{
			for (int j = 0; j < width; j++)
			{
				sum = 0;

				/* Add the second y derivative, if it exists */
				if (has_y_deriv(i,j))
				{
					sum +=  previous(i + 1, j) +
						previous(i - 1, j) -
						2*previous(i,j);
				}

				/* Add the second x derivative, if it exists */
				if (has_x_deriv(i,j))
				{
					sum +=  previous(i, j + 1) +
						previous(i, j - 1) -
						2*previous(i, j);
				}

				current(i, j) = previous(i,j) + coeff*sum;
			}
		}

		/* Wait for other threads */
		pthread_barrier_wait(&iteration_barrier);

		/* Once per round maintenance */
		if (id == 0)
		{
			/* Score swap */
			double * temp = u_previous;
			u_previous = u_current;
			u_current = temp;
		}
	}

	return NULL;
}
#endif

/* ~ The Main function ~ */
int main(int argc, char ** argv)
{
	/* Read commandline arguments */
	parse_args(argc, argv);

	printf("~~~ heateq, by Michael Sproul. 2013 ~~~\n");

	/* Calculate the differential equation coefficient, alpha */
	const double alpha = (CONDUCTIVITY/(DENSITY*SPEC_HEAT_CAP));

	/* TODO: Add the ability to change the material properties */

	/* If delta-t is too large the solution will be unstable! */
	{
		double dt_limit = (delta_s*delta_s)/(2*alpha);

		if (delta_t >= dt_limit)
		{
			printf("Please set a time interval less than %.5lf seconds\n",
								 dt_limit);
			exit(EXIT_FAILURE);
		}
	}

	/* Create two matricies of temperature values, current and previous */
	u_current = malloc(width*height*sizeof(double));
	u_previous = malloc(width*height*sizeof(double));

	if (u_current == NULL || u_previous == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	/* Set up the plate with some temperatures */
	assign_initial_temps();
	output_grid("initial.dat");

	printf("Material size: %dmm x %dmm\n", width, height);
	printf("α: %.6lf m²/s, Δt: %.5lf s\n", alpha, delta_t);
	printf("Iterations: %d\n", num_iterations);
	printf("Calculating...");

	/* Calculate the discretised heat equation coefficient */
	coeff = (alpha*delta_t)/(delta_s*delta_s);

	#ifndef _SEQUENTIAL
	/* Parallel Solution */
	int nthreads = 4;

	int quota = floor(1.00*height/nthreads);

	pthread_t threads[nthreads];
	struct work_chunk work[nthreads];

	for (int i = 0; i < nthreads; i++)
	{
		work[i].start = i*quota;
		work[i].end = (i + 1)*quota;
		work[i].id = i;
	}

	/* Set the last one to cover the remainder */
	work[nthreads - 1].end = height;

	/* Initialise the thread barrier */
	pthread_barrier_init(&iteration_barrier, NULL, nthreads);

	/* Create threads */
	for (int i = 0; i < nthreads; i++)
	{
		pthread_create(&threads[i], NULL, thread_calc, (void *)(work + i));
	}

	for (int i = 0; i < nthreads; i++)
	{
		pthread_join(threads[i], NULL);
	}

	/* Make u_current the actual current temps (unswap) */
	double * temp = u_previous;
	u_previous = u_current;
	u_current = temp;

	/* Destroy the thread barrier */
	pthread_barrier_destroy(&iteration_barrier);
	#endif

	#ifdef _SEQUENTIAL
	sequential_calc();
	#endif

	printf(" [done]\n");

	output_grid("final.dat");

	printf("Results written to initial.dat, final.dat\n");

	quit(EXIT_SUCCESS);
}
