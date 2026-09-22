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
 *    along with drm module if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"fft.h"
#include	<cstring>
/*
 */

	common_fft::common_fft (int32_t fft_size, bool dir) {
int32_t	i;

	this	-> fft_size	= fft_size;
	this	-> dir		= dir;

	vector	= (Complex *) FFTW_MALLOC (sizeof (Complex) * fft_size);
	for (i = 0; i < fft_size; i ++)
	   vector [i] = 0;
	plan	= FFTW_PLAN_DFT_1D (fft_size,
	                            reinterpret_cast <fftwf_complex *>(vector),
	                            reinterpret_cast <fftwf_complex *>(vector),
	                            FFTW_FORWARD, FFTW_ESTIMATE);
}

	common_fft::~common_fft () {
	   FFTW_DESTROY_PLAN (plan);
	   FFTW_FREE (vector);
}

void	common_fft::do_FFT (Complex *v) {
	if (dir)
	   for (int i = 0; i <fft_size; i ++)
	      vector [i] = conj (v [i]);
	else
	   for (int i = 0; i <fft_size; i ++)
	      vector [i] = v [i];
	FFTW_EXECUTE (plan);
	if (dir)
	   for (int i = 0; i < fft_size; i ++)
	      v [i] = conj (vector [i]);
	else
	for (int i = 0; i < fft_size; i ++)
	   v [i] = vector [i];
}

