#
/*
 *    Copyright (C) 2014 .. 2025
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm module
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

#pragma once

//
//	Simple wrapper around fftw
#include	"constants.h"
//
#define	FFTW_MALLOC		fftwf_malloc
#define	FFTW_PLAN_DFT_1D	fftwf_plan_dft_1d
#define FFTW_DESTROY_PLAN	fftwf_destroy_plan
#define	FFTW_FREE		fftwf_free
#define	FFTW_PLAN		fftwf_plan
#define	FFTW_EXECUTE		fftwf_execute
#include	<fftw3.h>
/*
 *	a simple wrapper
 */

class	common_fft {
public:
			common_fft	(int32_t);
			~common_fft	();
	void		do_FFT		(Complex *);
	void		do_IFFT		(Complex *);
	void		do_Shift	();
private:
	int32_t		fft_size;
	Complex		*vector;
	Complex		*vector1;
	FFTW_PLAN	plan;
	void		Scale		(Complex *);
};

class	common_ifft {
public:
			common_ifft	(int32_t);
			~common_ifft	();
	Complex		*getVector	();
	void		do_IFFT		();
private:
	int32_t		fft_size;
	Complex	*vector;
	FFTW_PLAN	plan;
	void		Scale		(Complex *);
};


