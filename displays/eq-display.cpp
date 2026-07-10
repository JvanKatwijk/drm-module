#
/*
 *    Copyright (C)  2016 .. 2024
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the Qt-DAB 
 *
 *    Qt-DAB is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    Qt-DAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Qt-DAB; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"eq-display.h"
#include	<QSettings>
#include	"constants.h"

	eqDisplay::eqDisplay (clickableChart *plotGrid,
	                      QSettings	*s):
	                         basicScope (plotGrid, s, "eqDisplay") {
	this	-> eqSettings	= s;
}

	eqDisplay::~eqDisplay	() {
}

void	eqDisplay::show_pilots	(const std::complex<float> *V, int amount) {
float	max	= 0;
double Y1_values [amount];
double Y2_values [amount];

	for (int i = 0; i < amount; i ++) {
	   Y1_values [i] = abs (V [i]);
	   if (abs (V [i]) > max)
	      max = abs (V [i]);
	}
	for (int i = 0; i < amount; i ++) 
	   Y2_values [i] = max / 2 + arg (V [i]) / M_PI;
	
	Y1_values [0]		= 0;
	Y1_values [amount - 1]	= 0;
	showSpectrum (Y1_values, Y2_values,
	              amount,
	              0, amount, 0, max);
}

void	eqDisplay::show_channel	(const std::complex<float> *V, int amount) {
	show_pilots (V, amount);
}

