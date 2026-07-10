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
 *    the Free Software Foundation: version 2 of the License;
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
#include	<stdio.h>
#include	<stdlib.h>
//#include	<Windows.h>
#include	<math.h>
#include	"radio.h"
#include	"basics.h"
#include	"word-collector.h"
#include	"reader.h"

//	The frequency shifter is in steps of 0.01 Hz
	wordCollector::wordCollector (RadioInterface *m_form,
	                              Reader	*b,
	                              smodeInfo	*modeInf,
	                              int sampleRate):
	                                 theShifter (100 * sampleRate),
	                                 fft (Tu_of (modeInf -> Mode)) {
	this	-> m_form	= m_form;
	this	-> buffer	= b;
	this	-> sampleRate	= sampleRate;
	this	-> modeInf	= modeInf;
	this	-> Mode		= modeInf -> Mode;
	this	-> Spectrum	= modeInf -> Spectrum;
	this	-> theAngle	= 0;
	this	-> Tu		= Tu_of (Mode);
	this	-> Ts		= Ts_of (Mode);
	this	-> Tg		= Tg_of (Mode);
	this	-> K_min	= Kmin	(Mode, Spectrum);
	this	-> K_max	= Kmax	(Mode, Spectrum);
	this	-> displayCount	= 0;
	connect (this, SIGNAL (show_fineOffset (float)),
                 m_form, SLOT (show_fineOffset (float)));
        connect (this, SIGNAL (show_coarseOffset (float)),
                 m_form, SLOT (show_coarseOffset (float)));
	connect (this, SIGNAL (set_channel_3 (const QString &)),
	         m_form, SLOT (set_channel_3 (const QString &)));
}

		wordCollector::~wordCollector () {
}

static	int counter	= 0;
static int amount	= 0;

void	wordCollector::getWord (Complex		*out,
	                        int32_t		offsetInteger,
	                        float		offsetFractional,
	                        float		freqOffset_fractional) {
Complex *temp  = dynVec (Complex, Ts);
int	f	= buffer -> currentIndex;

	buffer		-> waitfor (Ts + Ts / 2);
	theAngle	= freqOffset_fractional;

//	correction of the time offset by interpolation
	for (int i = 0; i < Ts; i ++) {
	   Complex one = buffer -> data [(f + i) % buffer -> bufSize];
	   Complex two = buffer -> data [(f + i + 1) % buffer -> bufSize];
	   temp [i] = one * (float)(1 - offsetFractional) +
                                 two * (float)(offsetFractional);
	}

//	And we shift the bufferpointer here
	buffer -> currentIndex = (f + Ts) & buffer -> bufMask;
//	offset  (and shift) in Hz / 100
	float offset		= theAngle / (2 * M_PI) * 100 * sampleRate / Tu;
	if (!isnan (offset))  // precaution to handle undefines
	   theShifter. do_shift (temp, Ts,
	                            100 * offsetInteger - offset);
	else
	   theAngle = 0;

	if (++displayCount > 20) {
	   displayCount = 0;
	   show_coarseOffset	(offsetInteger);
	   show_fineOffset	(offset / 100);
	}

	amount	= 0;

	counter	= 0;
	fft_and_extract (&temp [Tg], out);
}
//
//	The getWord as below is used in the main loop, to obtain
//	a next ofdm word
//
static int teller	= 0;
void	wordCollector::getWord (Complex		*out,
	                        int32_t		initialFreq,
	                        bool		firstTime,
	                        float		offsetFractional,
	                        float		angle,
	                        float		clockOffset) {
Complex* temp	=  dynVec (Complex, Ts);
int	f	= buffer -> currentIndex;

float	actOffset	= offsetFractional < 0 ? 1 + offsetFractional :
	                                          offsetFractional;
	buffer		-> waitfor (Ts + Ts / 2);
	teller ++;
	amount ++;
	if (amount >= 5) {
//	if (firstTime && amount > 100) {
	   buffer	-> waitfor (40 * Ts + Ts);
	   int intOffs	= get_intOffset (0 * Ts, 30, 10);
	   int sub	= get_intOffset (4 * Ts, 30, 10);
	   int sub_2	= get_intOffset (8 * Ts, 30, 10);

	   if ((intOffs == sub) && (sub == sub_2) && (sub != 0))  {
	      if (intOffs < -2) {
	         std::string str = std::to_string (teller);
			 str = str + "  " + std::to_string(intOffs);
	         set_channel_3 (QString::fromStdString (str));
	         amount = 0;
	         teller = 0;
	         f += -1;
	         counter --;
//	         fprintf (stderr, "counter %d\n", counter);
	      }
	      else
	      if (intOffs > 2 ) {
	         std::string str = std::to_string(teller);
	         str = str + "  " + std::to_string(intOffs);
	         set_channel_3 (QString::fromStdString (str));
	         amount = 0;
	         teller = 0;
	         f +=  1;
	         counter ++;
//	         fprintf (stderr, "counter %d\n", counter);
	      }
	   }
	   else
	      amount --;
	}

	int realS = offsetFractional < 0 ? f - 1 : f;
//	int realS = f;

	for (int i = 0; i < Ts; i ++) {
	   Complex one = buffer -> data [(realS + i) % buffer ->  bufSize];
	   Complex two = buffer -> data [(realS + i + 1) % buffer -> bufSize];
	   temp [i] = one  * (float)(1 - actOffset) +
	                   two *(float)(actOffset);
//	   temp [i] = one;
	}

//	And we adjust the bufferpointer here
	buffer -> currentIndex = (f + Ts) & buffer -> bufMask;
//
//	corrector
	Complex ss = Complex (0, 0);
	for (int i = 0; i < Tg; i ++)
           ss += conj (temp [Tu + i]) * temp [i];
//	theAngle	= 0.9 * theAngle + 0.1 * arg (ss);
	theAngle	= theAngle - 0.2 * angle;
	if (theAngle < -M_PI) {
		theAngle += M_PI;
		modeInf->freqOffset_integer -= sampleRate / Tu;
	}
	if (theAngle >= M_PI) {
	   theAngle -= M_PI;
	   modeInf->freqOffset_integer += sampleRate / Tu;
	}
//	offset in 0.01 * Hz
	float fineOffset   = theAngle / (2 * M_PI) * 100 * sampleRate / Tu;
	if (!isnan(fineOffset))  // precaution to handle undefines
	   theShifter. do_shift (temp, Ts,
	                        100 * modeInf -> freqOffset_integer - fineOffset);
	else
	   theAngle = 0;

	if (++displayCount > 20) {
	   displayCount = 0;
	   show_coarseOffset	(modeInf -> freqOffset_integer);
	   show_fineOffset	(- fineOffset / 100);
	}

	fft_and_extract (&temp [Tg], out);
}

void	wordCollector::fft_and_extract (Complex *in, Complex *out) {
//	and extract the Tu set of samples for fft processsing

	fft. do_FFT (in);
//	extract the "useful" data
	if (K_min < 0) {
	   memcpy (out,
	           &in [Tu + K_min],
	           - K_min * sizeof (Complex));
	   memcpy (&out [- K_min],
	           &in [0], (K_max + 1) * sizeof (Complex));
	}
	else
	   memcpy (out,
	           &in [K_min],
	           (K_max - K_min + 1) * sizeof (Complex));
}
//
int	wordCollector::get_intOffset	(int base,
	                                 int nSymbols,
	                                 int range) {
int	bestIndex = -1;
double	min_mmse = 10E20;

	for (int i = - range / 2; i < range / 2; i ++) {
	   int index = buffer -> currentIndex + base + i;
	   double mmse = compute_mmse (index, nSymbols);
	   if (mmse < min_mmse) {
	      min_mmse = mmse;
	      bestIndex = i;
	   }
	}
	
	return bestIndex;
}

double	wordCollector::compute_mmse (int starter,
	                             int nSymbols) {
Complex gamma = Complex (0, 0);
double	squares = 0;
int32_t		bufMask	= buffer -> bufSize - 1;

	buffer -> waitfor (nSymbols * Ts + Ts);
	for (int i = 0; i < nSymbols; i ++) {
	   int startSample = starter + i * Ts;
	   for (int j = 0; j < Tg; j ++) {
	      Complex f1 = buffer -> data [(startSample + j) & bufMask];
	      Complex f2 = buffer -> data [(startSample + Tu + j) & bufMask];
	      gamma	+= f1 * conj (f2);
	      squares	+= real (f1 * conj (f1)) + real (f2 * conj (f2));
	   }
	}
	return abs (squares - 2 * abs (gamma));
}
