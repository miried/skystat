/*
* visual.c - visual output
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

void	VIS_PlotEllipse( gnuplot_ctrl *plotCtrl ) {

	gnuplot_cmd( plotCtrl, "set xrange [-180:180]" );
	gnuplot_cmd( plotCtrl, "set yrange [-90:90]" );
	gnuplot_cmd( plotCtrl, "set xtics 30" );
	gnuplot_cmd( plotCtrl, "set ytics 30" );
	gnuplot_cmd( plotCtrl, "set multiplot" );
	gnuplot_cmd( plotCtrl, "set parametric" );
	gnuplot_cmd( plotCtrl, "plot 180*cos(t),90*sin(t) notitle" );
	gnuplot_cmd( plotCtrl, "unset parametric" );
}

void	VIS_DrawPlot( const char *buf ) {
	int		i;
	char		type;
	char		subType;
	gnuplot_ctrl	*plotCtrl;
	catalog_t	*cat;
	double		*x, *y;
	char		*caption;
	int		galactic;

	type = *buf;
	subType = *(buf+1);
	galactic = 0;

	switch (type) {
		case 'n':
		if ( subType == 'a' ) {
			cat = &nvss_A;
			caption = "NVSS A";
		}
		else if ( subType == 'b' ) {
			cat = &nvss_B;
			caption = "NVSS B";
		}
		else if ( subType == 'g' ) {
			galactic = 1;
			cat = nvss_culled;
			caption = "NVSS galactic";
		}
		else {
			cat = nvss_culled;
			caption = "NVSS";
		}
		break;
		case 's':
		if ( subType == 'a' ) {
			cat = &sdss_A;
			caption = "SDSS A";
		}
		else if ( subType == 'b' ) {
			cat = &sdss_B;
			caption = "SDSS B";
		}
		else {
			cat = sdss_culled;
			caption = "SDSS";
		}
		break;
		default:
			printf( "Unrecognized Plot type '%c'.\n", type );
			return;
	}

	// Initialize the Plot data
	plotCtrl = gnuplot_init();
	if ( plotCtrl == NULL )
		return;

	// output to file
	gnuplot_cmd(plotCtrl, "set terminal png size 800,400");
	gnuplot_cmd(plotCtrl, "set output \"doc/output.png\"");

	x = malloc( cat->number * sizeof(double) );
	y = malloc( cat->number * sizeof(double) );

	if ( galactic == 0 )
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			*(x+i) = -( cd->ra-180 ) * cos( cd->mollw_angle );
			//*(x+i) /= 90.0;
			*(y+i) = 90*sin( cd->mollw_angle );
		}
	else
		for (i=0;i<cat->number; i++ ) {
			catdata_t *cd;

			cd = cat->data+i;
			*(x+i) = -cd->longitude * cos( cd->mollw_angle_gal );
			//*(x+i) /= 90.0;
			*(y+i) = 90*sin( cd->mollw_angle_gal );
		}

	// Draw the Plot
	VIS_PlotEllipse( plotCtrl );
	gnuplot_plot_xy( plotCtrl, x, y, cat->number, caption );

	// Wait for user interaction
	getchar();
	// Close
	gnuplot_close( plotCtrl );
	free( x );
	free( y );
}

