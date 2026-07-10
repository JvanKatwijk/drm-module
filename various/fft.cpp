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

	common_fft::common_fft (int32_t fft_size) {
int32_t	i;

	this	-> fft_size = fft_size;

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
	for (int i = 0; i <fft_size; i ++)
	   vector [i] = v [i];
	FFTW_EXECUTE (plan);
	for (int i = 0; i < fft_size; i ++)
	   v [i] = vector [i];
}

/*
 * 	and a wrapper for the inverse transformation
 */
	common_ifft::common_ifft (int32_t fft_size) {
int32_t	i;

//	if ((fft_size & (fft_size - 1)) == 0)
	   this	-> fft_size = fft_size;
//	else
//	   this -> fft_size = 4096;	/* just a default	*/

	vector	= (Complex *)FFTW_MALLOC (sizeof (Complex) * fft_size);
	for (i = 0; i < fft_size; i ++)
	   vector [i] = 0;
	plan	= FFTW_PLAN_DFT_1D (fft_size,
	                            reinterpret_cast <fftwf_complex *>(vector),
	                            reinterpret_cast <fftwf_complex *>(vector),
	                            FFTW_BACKWARD, FFTW_ESTIMATE);
}

	common_ifft::~common_ifft () {
	   FFTW_DESTROY_PLAN (plan);
	   FFTW_FREE (vector);
}

Complex	*common_ifft::getVector () {
	return vector;
}

void	common_ifft::do_IFFT () {
	FFTW_EXECUTE	(plan);
	Scale		(vector);
}

void	common_ifft::Scale (Complex *Data) {
const float  Factor = 1.0 / float (fft_size);
int32_t	Position;

	// scale all entries
	for (Position = 0; Position < fft_size; Position ++)
	   Data [Position] *= Factor;
}


