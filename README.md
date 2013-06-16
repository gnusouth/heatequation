Heat Equation
============

A numerical solution to the heat equation in 2 dimensions, by Michael Sproul (C) 2013

### Usage

	heateq [-s widthxheight] [-I iterations] [-t delta-t]

	e.g.
	heateq -s 2000x400 -i 10000 -t 0.0001

width and height should both be given in millimeters.

delta-t, measured in seconds, controls how often the temperature at each point should be recalculated.

All parameters are optional and the default value for an option will be used if none is provided.

### Licensing
This code is licensed under the GNU GPLv3, see http://www.gnu.org/licenses/gpl.html

Modification and reuse permitted, so long as authorship is acknowledged and this license is preserved. No oil companies. 
