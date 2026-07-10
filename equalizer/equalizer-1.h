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
#pragma once

#include	"constants.h"
#include	<QObject>
#include	"basics.h"
#include	"equalizer-base.h"
#include	"ringbuffer.h"
#include	<vector>
#include	"my-array.h"

class		estimator_2;
class		RadioInterface;

typedef	struct {
	int16_t	symbol;
	int16_t	carrier;
} trainer;

class	equalizer_1: public QObject, public equalizer_base {
Q_OBJECT
public:
			equalizer_1 	(RadioInterface	*,
	                                 uint8_t	Mode,
	                                 uint8_t	Spectrum,
	                                 RingBuffer<Complex> *);
			~equalizer_1 	();	
	void		makeTheta	(float *, int, int, float, float);
	void		makePhi		(float **, int, float, float);
	bool		equalize	(Complex *,
	                                 int16_t,
	                                 myArray<theSignal>*,
	                                 std::vector<Complex> &);
	bool		equalize	(Complex *,
	                                 int16_t,
	                                 myArray<theSignal>*,
	                                 float *,
	                                 float *,
	                                 float *,
	                                 std::vector<Complex> &);
	void		set_scopeMode	(int);
private:
	RingBuffer<Complex> *eqBuffer;
	void		getRelAddress	(int16_t, int16_t *, int16_t *);
	int16_t		buildTrainers	(int16_t);
	std::complex<float>
                        build_corrector (int symbol, int carrier,
                                         int windowBase, int realWindow);

	int16_t		rndcnt;
	estimator_2	*estimator_channel;
	estimator_2	**estimators;
	int16_t		windowsinFrame;
	int16_t		periodforPilots;
	int16_t		periodforSymbols;
	double		**W_symbol_blk [20];
	float		f_cut_t;
	float		f_cut_k;
	std::vector<std::vector<trainer>> theTrainers;
	std::vector<std::vector<Complex>> pilotEstimates;
	int16_t		trainers_per_window [10];
	int16_t		symbols_to_delay;
	int16_t		symbols_per_window;
	void		processSymbol	(int16_t,
	                                 theSignal *,
	                                 std::vector<Complex> &);
	int16_t		Ts;
	int16_t		Tu;
	int16_t		Tg;

	float		sigmaq_noise;
	int		scopeMode;
signals:
	void		show_eqsymbol	(int);
};

