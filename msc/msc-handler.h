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
 *    drm moduleis distributed in the hope that it will be useful,
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

#include	"basics.h"
#include	<cstring>

class	stateDescriptor;
class	prbs;
class	RadioInterface;

//
//	empty base class for the different msc Handlers

class	mscHandler {
public:
		mscHandler 	(RadioInterface *, stateDescriptor *);
virtual		~mscHandler	();
virtual	void	process		(theSignal *, uint8_t *);
protected:
	stateDescriptor	*theState;
	int16_t		muxLength;
	prbs*		thePRBS;
	RadioInterface	*m_form;
};


