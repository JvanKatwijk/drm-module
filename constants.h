#
/*
 *    Copyright (C)  2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the sdrconnect drm module
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

#pragma once

#include	<math.h>
#include	<complex>
#include	<vector>
#include	<stdint.h>
#include	<limits>
#include	<stdlib.h>
#include	<unistd.h>
#ifdef __MINGW32__
#include        "windows.h"
#else
#include        "alloca.h"
#include        "dlfcn.h"
#endif
#ifndef M_PI
# define M_PI           3.14159265358979323846  /* pi */
#endif

#define dynVec(t, v)    static_cast<t *>(alloca (v * sizeof (t)))
//
//
typedef	std::complex<float>	Complex;

using namespace std;

#define	Hz(x)		(x)
#define	Khz(x)		(x * 1000)
#define	KHz(x)		(x * 1000)
#define	kHz(x)		(x * 1000)
#define	Mhz(x)		(Khz (x) * 1000)
#define	MHz(x)		(KHz (x) * 1000)
#define	mHz(x)		(KHz (x) * 1000)
/*
 */
#define	MINIMUM(x, y)	((x) < (y) ? x : y)
#define	MAXIMUM(x, y)	((x) > (y) ? x : y)

#define	CURRENT_VERSION	"0.8"

#define WORKING_RATE    12000
#define FILTER_DEFAULT  21

static inline  
float   compute_avg     (float oldVal, float newVal, float Alpha) {
        return (1 - Alpha) * oldVal + Alpha * newVal;
}

