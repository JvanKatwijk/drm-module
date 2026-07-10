#
/*
 *    Copyright (C) 2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm module for sdrconenct
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

#include	"constants.h"
#include	<QObject>
#include	"basics.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<cstring>
#include	<vector>
#define  _USE_MATH_DEFINES
#include        <math.h>
#include	"drm-shifter.h"
#include	"fft.h"

class	Reader;
class	RadioInterface;

class	freqSyncer: public QObject {
Q_OBJECT
public:
			freqSyncer 		(Reader *,
	                                         smodeInfo *,
	                                         int32_t,
	                                         RadioInterface *);

			~freqSyncer 		();
	bool		frequencySync 		(smodeInfo *);
private:
	int16_t		getWord			(Complex	*,
	                                         int32_t,
	                                         int32_t,
	                                         int16_t,
	                                         int,
	                                         float);
	int32_t		get_zeroBin 		(int16_t);
	float		get_spectrumOccupancy	(uint8_t,
	                                         int16_t);
	common_fft	fft;
	RadioInterface	*m_form;
	Reader		*theReader;
	int32_t		sampleRate;
	uint8_t		Mode;
	uint8_t		Spectrum;
	int16_t		k_pilot1;
	int16_t		k_pilot2;
	int16_t		k_pilot3;
	float		theAngle;
	int16_t		Tu;
	int16_t		Ts;
	int16_t		Tg;
	int16_t		displayCount;
	int16_t		nrSymbols;
	int16_t		bufferIndex;
	Complex		**symbolBuffer;
	Complex		corrV;
	drmShifter	theShifter;
signals:
	void		show_fineOffset (float);
};


