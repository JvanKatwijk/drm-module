#
/*
 *    Copyright (C)   2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the ft8 module
 *
 *    ft8 module is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    ft8 module is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with ft8 module; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#pragma once

#include	<QObject>
#include	<QString>
#include	<complex>
#include	<atomic>
#include	"constants.h"
#include	"ringbuffer.h"
#include	"socket-handler.h"
#include	"lowpassfilter.h"
#include	"down-converter.h"
#include	"oscillator.h"
#include	"iq-filter.h"

#define DEVICE_RATE     125000
#define	CONV_RATE	120000
#define	WORKING_RATE	12000

#define DIVIDER 1000
#define CONV_SIZE       (DEVICE_RATE / DIVIDER)

class messageHandler: public QObject {
Q_OBJECT
public:
		messageHandler (RingBuffer<Complex> *, int);
		~messageHandler	();
	void	setVFOFrequency	(int32_t);
	int	getVFOFrequency	();
	void	tryConnect	(const QString &, int);

	void	set_iqSelect	(bool);
private:
	oscillator	theOscillator;
	lowpassFIR	theFilter;
	new_downConverter	theDecimator;
	RingBuffer<std::complex<int16_t>> _I_Buffer;
	iqFilter		the_iqFilter;
	RingBuffer<Complex> *_O_Buffer;
	socketHandler	*theSocket;
	void		iqStreamEnable		(bool);
	void		setProperty		(const QString, const QString);
	void		askProperty		(const QString);
	int		outputRate;
	int		theSamplerate;
	bool		runMode;
	int		vfo_frequency;
	int		center_frequency;
	void		set_filterBW		(uint32_t);
	Complex		convBuffer      [CONV_SIZE + 1];
	int		convIndex;
	int16_t		mapTable_int    [CONV_RATE / DIVIDER];
	float		mapTable_float  [CONV_RATE / DIVIDER];

	float		dcReal;
	float		dcImag;
	bool		iqFilter_on;
private slots:
	void	connection_set		();
	void	no_connection		();
	void	binDataAvailable	();
	void	dispatchMessage		(const QString &);
	void	reportDisconnect	();
//	for local use
signals:
	void	connection_succeeded	();
	void	connection_failed	();
	void	frequency_changed	(int);
	void	signalPower		(double);
	void	dataAvailable		(int);
	void	set_disconnect		();
};

