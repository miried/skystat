/*
* math.c - mathematical computations
*
* Copyright (C) 2012 Michael Rieder <mr@student.ethz.ch>
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or (at
* your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

#include "skyplot.h"



/*
================
MAT_GreatCircD

great circle distance in units of radians (RAD)
================
*/
double	MAT_GreatCircD(double dra,double ddec,double cosdec1,double cosdec2) {
	double a, b, c;

	a = sin(ddec/2);
	b = sin(dra/2);

	c = sqrt(a*a + cosdec1*cosdec2*b*b);

	return 2*asin(c);
}

/*
================
MAT_AngDiamD

Angular diameter distance = comovD / (1+z)
================
*/
double	MAT_AngDiamD( double z, double comovD ) {
	double a;

	a = comovD / (1+z);

	return a;
}

/*
================
MAT_Mollweide

Calculate Mollweide Projection parameter iteratively
================
*/
void	MAT_Mollweide( double dec, double *result ) {
	double theta;
	double delta;

	theta = dec * RAD;
	// around the poles do nothing
	if ( fabs(theta) < 89.9*RAD )
		//iterate newton until error is small enough
		do {
			// newton for 2theta + sin(2theta) = pi*sin(dec)
			delta = 2*theta + sin(2*theta) - M_PI*sin(dec*RAD);
			delta /= 2 + 2*cos(2*theta);
			theta -= delta;
		} while ( fabs(delta) >= MOLLWEIDE_ERROR );

	// write back result
	*result = theta;
}

