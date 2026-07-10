#
/*
 *    Copyright (C) 2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    lazy Chair Computing
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
#include	"ringbuffer.h"
#include	"basics.h"
#include	<atomic>

class	RadioInterface;
//
//	For now we have a simple abstraction layer over a ringbuffer
//	that provides a suitable buffer.  It acts as a - more or
//	less regular - array, however, how the data gets in is kept
//	a secret. In a next version we eliminate the ringbuffer.
class	Reader {
public:
			Reader (RingBuffer<Complex> *, 
	                        uint32_t, RadioInterface *);
			~Reader		();
	void		waitfor		(int32_t);
	void		shiftBuffer	(int32_t);
	void		stop		();
	uint32_t	bufSize;
	uint32_t	bufMask;
	Complex		*data;
	uint32_t	currentIndex;
	bool		stopSignal;
	uint32_t	firstFreeCell;
	void		signal		();
private:
	RadioInterface	*m_form;
	uint32_t	Contents	();
	std::atomic<bool> theSignal;
	RingBuffer<Complex> * ringBuffer;
};


