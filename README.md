heateq, 2D Heat Equation Solver
============

A numerical solution to the heat equation in 2 dimensions, by Michael Sproul © 2013

### Theory

The heat equation is a partial differential equation describing how a thermal system will change in time. This program deals with the two dimensional case, described by:

∂u/∂t = α(∂²u/∂x² + ∂²u/∂y²)

α = k/cρ

k - Thermal conductivity in W per mK
c - Specific heat capacity in J per kgK
ρ - Density, in kg per m³

u(x, y, y) describes the temperature of the system at position (x,y) and time t.

The derivatives were discretised for computation, similar to the method used here: http://www.cosy.sbg.ac.at/events/parnum05/book/horak1.pdf

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
