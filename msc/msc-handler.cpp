#
/*
 *    Copyright (C) 2026
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
//	the real msc work is to be done by descendants of the mscHandler
//
#include	"msc-handler.h"
#include	"state-descriptor.h"
#include	"radio.h"

	mscHandler::mscHandler	(RadioInterface *m,
	                            stateDescriptor *theState) {
	this	-> m_form	= m;
	this	-> theState	= theState;
	muxLength		= theState	-> mscCells / 3;
}

	mscHandler::~mscHandler	() {
}

void	mscHandler::process	(theSignal *v, uint8_t * o) {
	(void)v; (void)o;
}


