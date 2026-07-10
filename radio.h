#
/*
 *    Copyright (C)  2026
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
#pragma once

#include	<QSettings>
#include	<QObject>
#include	<QFrame>
#include	<thread>
#include	"super-frame.h"
#include	<mutex>
#include	<atomic>
#include	<stdint.h>
#include	"constants.h"

//      for the payload we have
#include	"ringbuffer.h"
#include	"drm-bandfilter.h"
#include	"decimator.h"
#include	"basics.h"
#include	"reader.h"
#include	"backend-controller.h"
#include	"state-descriptor.h"
#include	"my-array.h"

#include	"program-list.h"

class	eqDisplay;
class	iqDisplay;
class	QSettings;
class	QLineEdit;
class	bandPlan;
class	audioSink;

class	messageHandler;
#include	"ui_drmdecoder.h"


class RadioInterface: public superFrame, private Ui_drmdecoder {
Q_OBJECT
public:
		RadioInterface (QSettings *,
	                        const QString	&stationList,
	                        bandPlan	*my_bandPlan);
	       ~RadioInterface ();
	void			WorkerFunction	();
private:
	QSettings		*drmSettings;
	RingBuffer<Complex> inputData;
	drmBandfilter           passbandFilter;
        decimator		theDecimator;

	RingBuffer<Complex>	workBuffer;
	RingBuffer<Complex>	audioOut;
	RingBuffer<Complex>	iqBuffer;
        RingBuffer<Complex> 	eqBuffer;
	Reader			my_Reader;   // single instance during life
	backendController	my_backendController;
	stateDescriptor		theState;
	programList		thePresets;
	bandPlan		*my_bandPlan;
	messageHandler		*inputHandler;

	audioSink		*audioHandler;
	std::vector<int16_t>	outTable;
	int			audioRate;
	void		setupSoundOut   (QComboBox        *streamOutSelector,
                                         audioSink        *our_audioSink,
                                         int32_t          cardRate,
                                         std::vector<int16_t> &table);

	QLineEdit		*myLine;
	std::thread		*m_worker;	
	eqDisplay		*my_eqDisplay;
	iqDisplay		*my_iqDisplay;
	std::mutex		m_lock;
	std::mutex	        locker;
	void			processSample	(std::complex<float>);
	int			resample	(std::complex<float>,
                                                 std::complex<float> *);
	uint8_t			getSpectrum	(stateDescriptor *);
	void			getMode		(Reader *my_Reader,
	                                                 smodeInfo *m);
//
//
        std::atomic<bool>       running;

	bool		drmError;
	int		centerFrequency;
        int             VFOFRequency;
        int             selectedFrequency;
        int             Raw_Rate;
//
//
	int		scopeMode;
        int16_t         nSymbols;
        int32_t         sampleRate;
        int8_t          windowDepth;
        smodeInfo       modeInf;
	void		frequencySync		(Reader	*my_Reader,
	                                         smodeInfo *m);
	int16_t		sdcCells		(smodeInfo *);
	bool		isFACcell		(smodeInfo *, int16_t, int16_t);
	bool		isSDCcell		(smodeInfo *, int16_t, int16_t);
	bool		isDatacell		(smodeInfo *,
                                                 int16_t,
                                                 int16_t,
	                                         int16_t);

	void		set_sdcCells		(smodeInfo *);
	std::vector<sdcCell> sdcTable;
	bool		isFirstFrame		(stateDescriptor *);
	bool		isLastFrame		(stateDescriptor *);
	void		addtoSuperFrame		(smodeInfo *,
                                                 int16_t,
	                                         myArray<theSignal> *);

private slots:
	void		handle_hostName		();
//	void		reset			();
        void		handle_connection_failed		();
        void		handle_connection_succeeded		();
        void		sampleHandler		(int);

	void		select_channel_1	();
	void		select_channel_2	();
        void            handle_modeSelector     (const QString &);
	void		handle_reset		();
	void		setStreamOutSelector	(int idx);

public slots:
	void		handle_presetButton	();
	void		handle_saveFreqButton	();
	void		handle_myLine		();
	void		setFrequency		(int32_t);
	void		report_vfoFrequency	(int32_t);
	void            set_faadSyncLabel	(bool);
	void		set_messageLabel	(const QString &);
	void		set_audioModeLabel	(const QString &);
	void		set_aacDataLabel	(const QString &);
	void		set_timeLabel		(const QString &);

	void		show_fineOffset		(float);
	void		show_coarseOffset	(float);
	void		executeTimeSync		(bool);
	void		executeFACSync		(bool);
	void		executeSDCSync		(bool);
	void		execute_showMode	(int);
	void		execute_showSpectrum	(int);

	void		show_fac_mer		(float);
	void            show_sdc_mer		(float);
	void		show_msc_mer		(float);

	void		set_datacoding		(const QString &);

	void		set_channel_1		(const QString &);
	void		set_channel_2		(const QString &);
	void		set_channel_3		(const QString &);
	void		set_channel_4		(const QString &);
	void		audioAvailable		();
	void		show_eqsymbol		(int);
	void		showIQ			(int);
	void		handle_quit		();

	void		show_signalPower	(double);
signals:
	void		audioAvailable		(int, int);
	void		setTimeSync		(bool);
        void		setFACSync		(bool);
        void		setSDCSync		(bool);
        void		show_Mode		(int);
        void		show_Spectrum		(int);
};
