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
 *    the Free Software Foundation,  version 2 of the License;
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
#include	"basics.h"

bool		isTimeCell	(uint8_t, int16_t, int16_t);
bool		isPilotCell	(uint8_t, int16_t, int16_t);
bool		isBoostCell	(uint8_t, uint8_t, int16_t);
bool		isFreqCell	(uint8_t, int16_t, int16_t);
Complex		getFreqRef	(uint8_t, int16_t, int16_t);
Complex		getTimeRef	(uint8_t, int16_t, int16_t);
Complex		getGainRef	(uint8_t, int16_t, int16_t);
Complex		getPilotValue	(uint8_t, uint8_t, int16_t, int16_t);
float		init_gain_ref_cells (int16_t *cells_k,
	                             Complex *cells_v, int16_t *cnt);


