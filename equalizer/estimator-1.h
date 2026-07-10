#
/*
 *    Copyright (C) 2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm module for sdrconnect
 *
 *    drm module is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    drm module is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with drm module; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#
#pragma once

#include	"referenceframe.h"

//	The processor for estimating the channel(s) of a single
//	symbol
class	estimator_1 {
public:
		estimator_1	(Complex **, uint8_t, uint8_t, int16_t);
		~estimator_1	();
	void	estimate	(Complex *, Complex *);
private:
	Complex		**refFrame;
	uint8_t		Mode;
	uint8_t		Spectrum;
	int16_t		refSymbol;
	int16_t		K_min;
	int16_t		K_max;
	int16_t		indexFor	(int16_t);
	int16_t		getnrPilots	(int16_t);
};

