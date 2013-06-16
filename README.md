heateq, 2D Heat Equation Solver
============

A numerical solution to the heat equation in 2 dimensions, by Michael Sproul (C) 2013

### Usage

	heateq [-s widthxheight] [-I iterations] [-t delta-t]

	e.g.
	heateq -s 2000x400 -i 10000 -t 0.0001

Width and height should both be given in millimeters. Delta-t, measured in seconds, controls how often the temperature at each point should be recalculated.

All parameters are optional and the default value for an option will be used if none is provided.

### Output

The program will output 2 text files containing the temperatures along the surface at the beginning and end of the computation.

The data can be visualised using the free program 'gnuplot' by running `make plot`, or a command of this form:

	gnuplot -e "splot 'filename' matrix; pause -1"


### Licensing
This code is licensed under the GNU GPLv3, see http://www.gnu.org/licenses/gpl.html

Modification and reuse permitted, so long as authorship is acknowledged and this license is preserved. No oil companies. 
