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
/*
 *	shield for mscProcessor 
 */
#pragma once

#include	<QObject>
#include	"basics.h"
#include	"ringbuffer.h"
#include	"constants.h"

class   stateDescriptor;
class	RadioInterface;
class	mscProcessor;

class	backendController: public QObject {
Q_OBJECT
public:
		backendController	(RadioInterface *,
	                                 int8_t,
	                                 RingBuffer<Complex> *,
	                                 RingBuffer<Complex> *);
		~backendController	();
	void	newFrame		(stateDescriptor *);
	void	addtoMux		(int16_t, int, theSignal);
	void	endofFrame		();
	void	reset			(stateDescriptor *);

private:
	RadioInterface	*m_form;
	mscProcessor	*theWorker;
	stateDescriptor	*theState;
	RingBuffer<Complex> *audioBuffer;
	RingBuffer<Complex> *iqBuffer;
	int8_t		qam64Roulette;
	uint8_t		mscMode;
	uint8_t		protLevelA;
	uint8_t		protLevelB;
	int16_t		numofStreams;
	uint8_t		QAMMode;
signals:
	void		showIQ		(int);
};

