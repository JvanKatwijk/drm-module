#
/*
 *    Copyright (C) 2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the sdrconnect drm module
 *   
 *    drm module is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
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
#include	<vector>
#include	<chrono>

#include	<QMessageBox>
#include	<QLineEdit>
#include	"radio.h"
#include	"message-handler.h"
#include	"audiosink.h"
//
//	drm specifics

#include	"utilities.h"
#include	"bandplan.h"
#include	"timesync.h"
#include	"freqsyncer.h"
#include	"word-collector.h"
#include	"correlator.h"
#include	"referenceframe.h"
#include	"equalizer-1.h"
#include	"fac-processor.h"
#include	"fac-tables.h"
#include	"sdc-processor.h"
#include	"eq-display.h"
#include	"iq-display.h"

#define  _USE_MATH_DEFINES
#include	<math.h>
#define	STEP	(INRATE / WORKING_RATE)
	RadioInterface::RadioInterface (QSettings	*s,
	                                const QString	&stationList,
	                                bandPlan	*my_bandPlan):
	                                  superFrame (nullptr),
	                                  inputData  (32 * 32768),
	                                  passbandFilter (11,
                                                         -8000,
                                                         +8000,
                                                         INRATE),
                                          theDecimator (STEP + 1,
	                                                -5500,
	                                                 5500,
	                                                 INRATE, STEP),
	                                  workBuffer  (32 * 32768),
	                                  audioOut (16 * 32768),
	                                  iqBuffer (32768),
	                                  eqBuffer (32768),
	                                  my_Reader (&workBuffer,
	                                               2 * 16384, this),
	                                  my_backendController (this, 4,
	                                                       &audioOut,
	                                                       &iqBuffer),
	                                  theState (1, 3),
	                                  thePresets (this, stationList),
	                                  m_worker (nullptr) {
	drmSettings		= s;
	setupUi (this);
	my_eqDisplay            = new eqDisplay (equalizerDisplay, s);
	my_iqDisplay            = new iqDisplay (iqPlotter, s);

	connect (this, &superFrame::frameClosed,
	         this, &RadioInterface::handle_quit);
	scopeMode		= SHOW_PILOTS;
	show ();
	running. store (false);

	myLine			= nullptr;
	this	-> my_bandPlan	= my_bandPlan;
	drmError		= false;
	nSymbols		= 25;
	modeInf. Mode		= 2;
	modeInf. Spectrum	= 3;

	inputHandler            = nullptr;
        drmSettings        -> beginGroup ("drmSettings");
        QString serverAddress
               = drmSettings -> value ("serverAddress", "127.0.0.1"). toString ();
        drmSettings        -> endGroup ();
        hostNameSelector   -> setInputMask ("000.000.000.000");
        hostNameSelector   -> setText (serverAddress);
        connect (startButton, &QPushButton::clicked,
                 this, &RadioInterface::handle_hostName);

//	output device
	this		-> audioRate	= 48000;

	audioHandler            = new audioSink (this -> audioRate, 16384);
	outTable. resize (audioHandler -> numberofDevices () + 1);
	for (int i = 0; i < audioHandler -> numberofDevices (); i ++)
	   outTable [i] = -1;

	try {
	   setupSoundOut (streamOutSelector, audioHandler,
	                  audioRate, outTable);
	   connect (streamOutSelector, &QComboBox::activated,
	            this, &RadioInterface::setStreamOutSelector);

	} catch (int e) {
	   QMessageBox::warning (nullptr, tr ("sdr"),
	                               tr ("Opening audio failed\n"));
	   abort ();
	}
	
	audioHandler -> selectDefaultDevice ();

}

	RadioInterface::~RadioInterface () {
	running. store (false);
	my_Reader. stop ();
	if (m_worker != nullptr) {
	   m_worker        -> join ();
	   delete 	m_worker;
	   m_worker = nullptr;
	}
	if (my_eqDisplay != nullptr)
	   delete	my_eqDisplay;
	my_eqDisplay	= nullptr;
	if (my_iqDisplay != nullptr)
	   delete	my_iqDisplay;
	my_iqDisplay	= nullptr;
	if (inputHandler != nullptr)
	   delete	inputHandler;
	inputHandler	= nullptr;
}
	
void    RadioInterface::handle_hostName         () {
        if (inputHandler != nullptr)
           return;
	inputHandler            = new messageHandler (&inputData, 7880000);
	connect (inputHandler, &messageHandler::connection_failed,
                 this, &RadioInterface::handle_connection_failed);
        connect (inputHandler, &messageHandler::connection_succeeded,
                 this, &RadioInterface::handle_connection_succeeded);
        hostNameSelector   -> setInputMask ("000.000.000.000");
        hostNameSelector   -> setText ("127.0.0.1");
        inputHandler    -> tryConnect (hostNameSelector -> text (),
                                       portSelector -> value ());
}

void	RadioInterface::handle_connection_failed () {
	running. store (false);
        QMessageBox::warning (nullptr, tr ("Warning"),
                                 tr ("connection failed"));

        disconnect (inputHandler, &messageHandler::connection_failed,
                    this, &RadioInterface::handle_connection_failed);
        disconnect (inputHandler, &messageHandler::connection_succeeded,
                    this, &RadioInterface::handle_connection_succeeded);
        if (inputHandler != nullptr)
           delete inputHandler;
}

void	RadioInterface::handle_connection_succeeded () {
	disconnect (startButton, &QPushButton::clicked,
	            this, &RadioInterface::handle_hostName);
	disconnect (inputHandler, &messageHandler::connection_succeeded,
	            this, &RadioInterface::handle_connection_succeeded);
        connectLabel    -> setText ("connected");

	connect (inputHandler, &messageHandler::dataAvailable,
                 this, &RadioInterface::sampleHandler);
	connect (inputHandler, &messageHandler::frequency_changed,
	         this, &RadioInterface::report_vfoFrequency);
	connect (inputHandler, &messageHandler::signalPower,
	         this, &RadioInterface::show_signalPower);
	connect (presetButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_presetButton);
	connect (saveFreqButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_saveFreqButton);
	connect (modeSelector, &QComboBox::textActivated,
                 this, &RadioInterface::handle_modeSelector);
	connect (this, SIGNAL (setTimeSync (bool)),
                 this, SLOT (executeTimeSync (bool)));
        connect (this, SIGNAL (setFACSync (bool)),
                 this, SLOT (executeFACSync (bool)));
        connect (this, SIGNAL (setSDCSync (bool)),
                 this, SLOT (executeSDCSync (bool)));
        connect (this, SIGNAL (show_Mode (int)),
                 this, SLOT (execute_showMode (int)));
        connect (this, SIGNAL(show_Spectrum (int)),
                 this, SLOT (execute_showSpectrum (int)));
	connect (channel_1, SIGNAL (clicked ()),
                 this, SLOT (select_channel_1 ()));
        connect (channel_2, SIGNAL (clicked ()),
                 this, SLOT (select_channel_2 ()));

	connect (resetButton, SIGNAL (clicked ()),
	         this, SLOT (handle_reset ()));
	m_worker	=
	       new std::thread (&RadioInterface::WorkerFunction, this);
}

//
void	RadioInterface::sampleHandler	(int amount) {
	(void)amount;
	while (inputData. GetRingBufferReadAvailable () > INRATE / 10) {
	   Complex sample;
	   inputData. getDataFromBuffer (&sample, 1);
	   sample	= passbandFilter. Pass (sample);
	   if (!theDecimator. Pass (sample, &sample))
	      continue;
	   workBuffer. putDataIntoBuffer (&sample, 1);
        }
}

void	RadioInterface::WorkerFunction () {
int16_t	blockCount      = 0;
bool	inSync;
int16_t	symbol_no       = 0;
bool	frameReady;
int counter = 0;
float     deltaFreqOffset         = 0;
float     sampleclockOffset       = 0;

	running. store (true);
	while (running. load ()) {
	   try {
	      if (!running. load ())
	         throw (21);
	      counter++;
	      emit setTimeSync (false);
	      emit setFACSync (false);
	      emit setSDCSync (false);
	      theState. cleanUp ();
	      my_Reader. waitfor (Ts_of (Mode_A));
	  
//      First step: find mode and starting point
	      modeInf. Mode = -1;
	      while (running. load () && (modeInf. Mode == -1)) {
	         my_Reader. shiftBuffer (Ts_of (Mode_A) / 3);
	         getMode (&my_Reader, &modeInf);
	      }

	      if (!running. load ())
	         throw (20);

	      setTimeSync (true);
	      my_Reader. shiftBuffer (modeInf. timeOffset_integer);
	      frequencySync (&my_Reader, &modeInf);

	      show_Mode		(modeInf. Mode);
	      show_Spectrum	(modeInf. Spectrum);
	      show_coarseOffset	(modeInf. freqOffset_integer);
	      show_fineOffset	(modeInf. freqOffset_fractional);

	      theState. Mode		= modeInf. Mode;
	      theState. Spectrum	= modeInf. Spectrum;
	      int nrSymbols		= symbolsperFrame (modeInf. Mode);
	      int nrCarriers       = Kmax (modeInf. Mode, modeInf. Spectrum) -
	                             Kmin (modeInf. Mode, modeInf. Spectrum) + 1;

	      myArray<std::complex<float>> inbank (nrSymbols, nrCarriers);
	      myArray<theSignal> outbank (nrSymbols, nrCarriers);
	      correlator myCorrelator (&modeInf);
	      equalizer_1 my_Equalizer (this,
	                                modeInf.Mode,
	                                modeInf.Spectrum,
	                                &eqBuffer);
	
	      my_Equalizer. set_scopeMode (scopeMode);
	      std::vector<std::complex<float>> displayVector;
	      displayVector. resize (Kmax (modeInf. Mode, modeInf. Spectrum) -
	                             Kmin (modeInf. Mode, modeInf. Spectrum) + 1);
//
//	The first step is to correlate to find the first 
//	word of the group
	      wordCollector my_wordCollector (this,
	                                      &my_Reader,
	                                      &modeInf,
	                                      WORKING_RATE);
	      facProcessor my_facProcessor (this, &modeInf);

//	   we know that - when starting - we are not "in sync" yet
	      inSync	= false;
//
//	   we read one full frame after which we start looking for a 
//	   match
	      for (int symbol = 0; symbol < nrSymbols - 1; symbol ++) {
	         my_wordCollector. getWord (inbank. element (symbol),
	                                    modeInf. freqOffset_integer,
	                                    modeInf. timeOffset_fractional,
	                                    modeInf. freqOffset_fractional
	                                   );
	         myCorrelator. correlate (inbank. element (symbol), symbol);
	       }

	      int  lc      = nrSymbols - 1;
//      We keep on reading here until we are satisfied that the
//      frame that is in, looks like a decent frame, just by the
//      correlation on the first word
	      while (running. load ()) {
	         my_wordCollector. getWord (inbank. element (lc),
	                                    modeInf. freqOffset_integer,
	                                    modeInf. timeOffset_fractional,
	                                    modeInf. freqOffset_fractional
	                                   );
	         myCorrelator. correlate (inbank. element (lc), lc);
	         lc = (lc + 1) % symbolsperFrame (modeInf. Mode);
	         if (myCorrelator. bestIndex (lc))  {
	            break;
	         }
	      }
		
//      from here on, we know that in the input bank, the frames occupy the
//      rows "lc" ... "(lc + symbolsinFrame) % symbolsinFrame"
//      so, once here, we know that the frame starts with index lc,
//      so let us equalize the last symbolsinFrame words in the buffer
	      for (symbol_no = 0; symbol_no < nrSymbols; symbol_no ++)
	         (void) my_Equalizer.
	            equalize (inbank. element ((lc + symbol_no) % nrSymbols),
	                      symbol_no,
	                      &outbank,
                              &modeInf. timeOffset_fractional,
                              &deltaFreqOffset,
                              &sampleclockOffset,
	                      displayVector);

	      lc           = (lc + symbol_no) % symbol_no;
	      symbol_no    = 0;
	      frameReady   = false;
	      while (running. load () && !frameReady) {
	         my_Equalizer. set_scopeMode (scopeMode);
		  my_wordCollector.
	                 getWord (inbank.element(lc),
				   modeInf.freqOffset_integer,
				   lc == 0,        // no-op
				   modeInf. timeOffset_fractional,
				   deltaFreqOffset,  // tracking value
				   sampleclockOffset); // tracking value

	         frameReady = my_Equalizer. 
	                            equalize (inbank. element (lc),
	                                      symbol_no,
	                                      &outbank,
	                                      &modeInf. timeOffset_fractional,
                                              &deltaFreqOffset,
                                              &sampleclockOffset,
	                                      displayVector);
			
	         lc = (lc + 1) % nrSymbols;
	         symbol_no = (symbol_no + 1) % nrSymbols;
	      }

	      if (!running.load())
	         throw (37);

//	when we are here, we do have  our first full "frame".
//	so, we will be convinced that we are OK when we have a decent FAC
	      inSync = my_facProcessor.  processFAC  (&outbank, &theState);

	  //	one test:
	      if (!inSync)
	         throw (33);
	      if (modeInf. Spectrum != getSpectrum (&theState))
	         throw (34);
	      emit setFACSync (true);
		
//
//	prepare for sdc processing
//	Since computing the position of the sdc Cells depends (a.o)
//	on FAC and other data cells, we better create the table here.
	      sdcTable. resize (sdcCells (&modeInf));
	      set_sdcCells (&modeInf);
	      sdcProcessor my_sdcProcessor (this, &modeInf,
	                                    sdcTable, &theState);

	      bool	superframer		= false;
	      int	missers			= 0;
	      bool	firstTime		= true;
	      float	deltaFreqOffset		= 0;
	      float	sampleclockOffset	= 0;
		  
		
	      while (true) {
	         my_Equalizer. set_scopeMode (scopeMode);
//	when we are here, we can start thinking about  SDC's and superframes
//	The first frame of a superframe has an SDC part
	         if (isFirstFrame (&theState)) {
	            bool sdcOK = my_sdcProcessor. processSDC (&outbank);
	            emit setSDCSync (sdcOK);
	            if (sdcOK) {
	               blockCount	= 0;
	            }
//
//	if we seem to have the start of a superframe, we
//	re-create a backend with the right parameters
	            if (!superframer && sdcOK)
	               my_backendController. reset (&theState);
	            superframer	= sdcOK;
	         }
//
//	when here, add the current frame to the superframe.
//	Obviously, we cannot garantee that all data is in order
	         if (superframer)
	            addtoSuperFrame (&modeInf, blockCount ++, &outbank);

//	when we are here, it is time to build the next frame
	         frameReady	= false;
	         for (int i = 0; !frameReady && (i < nrSymbols); i ++) {
	            my_wordCollector.
	               getWord (inbank. element ((lc + i) % nrSymbols),
	                        modeInf. freqOffset_integer,	// initial value
	                        (lc + i) % nrSymbols == 0,
	                        modeInf. timeOffset_fractional,
	                        deltaFreqOffset,	// tracking value
	                        sampleclockOffset	// tracking value
	                      );
	            firstTime	= false;
	            frameReady =
	                  my_Equalizer.
	                         equalize (inbank. element ((lc + i) % nrSymbols),
	                                   (symbol_no + i) % nrSymbols,
	                                   &outbank,
	                                   &modeInf. timeOffset_fractional,
	                                   &deltaFreqOffset,
	                                   &sampleclockOffset,
	                                   displayVector);
	         }
	
	         if (!frameReady)	// should not happen???
	            throw (36);
			 
//	OK, let us check the FAC
	         bool success  = my_facProcessor.
	                          processFAC (&outbank, &theState);
	         if (success) {
	            emit setFACSync	(true);
	            missers = 0;
	         }
	         else {
	            emit setFACSync	(false);
	            emit setSDCSync	(false);
	            superframer		= false;
	            if (missers++ < 2)
	               continue;
	            throw (35);	// ... or give up and start all over
	         }
	      }	// end of main loop
	   } catch (int e) {
	      if (!running. load ())
	         return;
	   }
	}
}

//      just for readability
uint8_t RadioInterface::getSpectrum     (stateDescriptor *f) {
uint8_t val = f -> spectrumBits;
	return val <= 5 ? val : 3;
}
//

void	RadioInterface::getMode (Reader *my_Reader, smodeInfo *m) {
timeSyncer  my_Syncer (my_Reader, WORKING_RATE,  nSymbols);
	my_Syncer. getMode (m);
}

void    RadioInterface::frequencySync (Reader *my_Reader, smodeInfo *m) {
freqSyncer my_Syncer (my_Reader, m, WORKING_RATE, this);
	my_Syncer. frequencySync (m);
}

int16_t	RadioInterface::sdcCells (smodeInfo *m) {
static
int m1_table []	= {167, 190, 359, 405, 754, 846};
static
int m2_table [] = {130, 150, 282, 322, 588, 1500};

	switch (m -> Mode) {
	   case Mode_A:
	      return m1_table [m -> Spectrum];

	   default:
	   case Mode_B:
	      return m2_table [m -> Spectrum];

	   case Mode_C:
	      return 288;

	   case Mode_D:
	      return 152;
	}
	return 288;
}

void	RadioInterface::set_sdcCells (smodeInfo *modeInf) {
uint8_t	Mode	= modeInf -> Mode;
uint8_t	Spectrum = modeInf -> Spectrum;
int	carrier;
int	cnt	= 0;
	for (carrier = Kmin(Mode, Spectrum);
	     carrier <= Kmax(Mode, Spectrum); carrier++) {
	   if (isSDCcell (modeInf, 0, carrier)) {
		  sdcTable[cnt].symbol = 0;
		  sdcTable[cnt].carrier = carrier;
		  cnt++;
	   }
	}
	
	for (carrier = Kmin (Mode, Spectrum);
	     carrier <= Kmax (Mode, Spectrum); carrier ++) {
	   if (isSDCcell (modeInf, 1, carrier)) {
	      sdcTable [cnt]. symbol = 1;
	      sdcTable [cnt]. carrier = carrier;
	      cnt ++;
	   }
	}
	   
	if ((Mode == Mode_C) || (Mode == Mode_D)) {
	   for (carrier = Kmin (Mode, Spectrum);
	        carrier <= Kmax (Mode, Spectrum); carrier ++) 
	      if (isSDCcell (modeInf, 2, carrier)) {
	         sdcTable [cnt]. symbol = 2;
	         sdcTable [cnt]. carrier = carrier;
	         cnt ++;
	      }
	}

	fprintf (stderr, "for Mode %d, spectrum %d, we have %d sdc cells\n",
	                       Mode, Spectrum, cnt);
}

bool	RadioInterface::isFACcell (smodeInfo *m,
	                             int16_t symbol, int16_t carrier) {
int16_t	i;
struct facElement *facTable     = getFacTableforMode (m -> Mode);

//	we know that FAC cells are always in positive carriers
	if (carrier < 0)
	   return false;
	for (i = 0; facTable [i]. symbol != -1; i ++) {
	   if (facTable [i]. symbol > symbol)
	      return false;
	   if ((facTable [i]. symbol == symbol) &&
	       (facTable [i]. carrier == carrier))
	      return true;
	}
	return false;
}

bool	RadioInterface::isSDCcell (smodeInfo *m,
	                             int16_t symbol, int16_t carrier) {
	if (carrier == 0)
	   return false;
	if ((m -> Mode == 1) && ((carrier == -1) || (carrier == 1)))
	   return false;

	if (symbol > 2)
	   return false;
	if (isTimeCell (m -> Mode, symbol, carrier))
	   return false;
	if (isFreqCell (m -> Mode, symbol, carrier))
	   return false;
	if (isPilotCell (m -> Mode, symbol, carrier))
	   return false;
	if (isFACcell (m, symbol, carrier))
	   return false;
	return true;
}

bool	RadioInterface::isDatacell (smodeInfo *m,
	                              int16_t symbol,
	                              int16_t carrier, int16_t blockno) {
	if (carrier == 0)
	   return false;
	if (m -> Mode == 1 && (carrier == -1 || carrier == 1))
	   return false;
//
//	these are definitely SDC cells
	if ((blockno == 0) && ((symbol == 0) || (symbol == 1)))
	   return false;
//
	if ((blockno == 0) && ((m -> Mode == 3 ) || (m -> Mode == 4)))
	   if (symbol == 2)
	      return false;
	if (isFreqCell (m -> Mode, symbol, carrier))
	   return false;
	if (isPilotCell (m -> Mode, symbol, carrier))
	   return false;
	if (isTimeCell (m -> Mode, symbol, carrier))
	   return false;
	if (isFACcell (m, symbol, carrier))
	   return false;

	return true;
}

bool	RadioInterface::isFirstFrame (stateDescriptor *f) {
uint8_t val     = f -> frameIdentity;
	return ((val & 03) == 0) || ((val & 03) == 03);
}

bool	RadioInterface::isLastFrame (stateDescriptor *f) {
uint8_t val     = f -> frameIdentity;
	return ((val & 03) == 02);
}

//
//	adding the contents of a frame to a superframe
void	RadioInterface::addtoSuperFrame (smodeInfo *m,
	                                 int16_t blockno, 
	                                 myArray<theSignal> *outbank) {
static	int	teller	= 0;
int16_t	symbol, carrier;
int16_t	   K_min		= Kmin (m -> Mode, m -> Spectrum);
int16_t	   K_max		= Kmax (m -> Mode, m -> Spectrum);

	if (theState. numofStreams <= 0) 
	   return;

	if (isFirstFrame (&theState))
	   my_backendController. newFrame (&theState);

	for (symbol = 0; symbol < symbolsperFrame (m -> Mode); symbol ++) {
	   for (carrier = K_min; carrier <= K_max; carrier ++)
	      if (isDatacell (&modeInf, symbol, carrier, blockno)) {
	         my_backendController. addtoMux (blockno, teller ++,
	                                     outbank -> element (symbol)[carrier - K_min]);
	      }
	}

	if (isLastFrame (&theState)) {
	   my_backendController. endofFrame ();
	   teller = 0;
	}
}

void	RadioInterface::executeTimeSync	(bool f) {
	if (f)
	   timeSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
	else {
	   timeSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	   faadSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	}
}

void	RadioInterface::executeFACSync	(bool f) {
	if (f)
	   facSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
	else {
	   facSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	   faadSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	}
}

void	RadioInterface::executeSDCSync	(bool f) {
	if (f)
	   sdcSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
	else {
	   sdcSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	   faadSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	}
}

void    RadioInterface::execute_showMode            (int l) {
        if (1 <= l && l <= 4)
           modeIndicator        -> setText (QString (char ('A' + (l - 1))));
}

void    RadioInterface::execute_showSpectrum        (int l) {
        if (0 <= l && l < 6)
           spectrumIndicator    -> setText (QString (char ('0' + l)));
}


void    RadioInterface::set_audioModeLabel	(const QString &s) {
        audioModelabel  -> setText (s);
}

void	RadioInterface::show_fineOffset	(float f) {
	show_small_offset -> display (f);
}

void	RadioInterface::show_coarseOffset	(float f) {
	show_int_offset	-> display (f);
}

void	RadioInterface::set_faadSyncLabel	(bool b) {
static  int     faadCounter     = 0;
static  int     goodfaad        = 0;
        faadCounter ++;
        if (b)
           faadSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
        else
           faadSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
        if (b)
           goodfaad ++;
        if (faadCounter > 500) {
           float ratio = goodfaad / 500.0;
           channel_4 -> setText (QString::number (ratio));
           goodfaad     = 0;
           faadCounter  = 0;
        }
}

void	RadioInterface::set_messageLabel	(const QString &s) {
	messageLabel	-> setText (s);
}

void	RadioInterface::set_timeLabel	(const QString &s) {
	timeLabel	-> setText (s);
}

void	RadioInterface::show_fac_mer	(float f) {
	fac_mer	-> display (f);;
}

void	RadioInterface::show_sdc_mer	(float f) {
	sdc_mer -> display (f);
}

void    RadioInterface::show_msc_mer        (float f) {
        msc_mer -> display (f);
}

void	RadioInterface::set_aacDataLabel	(const QString &s) {
	aacDataLabel	-> setText (s);
}

void	RadioInterface::set_channel_1	(const QString &s) {
	channel_1	-> setText (s);
	selectedService	-> setText (s);
}

void	RadioInterface::set_channel_2	(const QString &s) {
	channel_2	-> setText (s);
}

void	RadioInterface::set_channel_3	(const QString &s) {
	channel_3	-> setText (s);
}

void	RadioInterface::set_channel_4	(const QString &s) {
	channel_4	-> setText (s);
}

void	RadioInterface::set_datacoding	(const QString &s) {
	datacoding	-> setText (s);
}

void	RadioInterface::audioAvailable	() {
Complex buffer [512];
	while (audioOut. GetRingBufferReadAvailable () > 512) {
	   audioOut. getDataFromBuffer (buffer, 512);
	   audioHandler -> putSamples (buffer, 512);
	}
}

void    RadioInterface::show_eqsymbol       (int amount) {
std::complex<float> line [amount];

	eqBuffer. getDataFromBuffer (line, amount);
        if (scopeMode == SHOW_PILOTS)
           my_eqDisplay    -> show_pilots (line, amount);
        else
	if (scopeMode == SHOW_CHANNEL)
           my_eqDisplay    -> show_channel (line, amount);
	else
	   my_eqDisplay	 -> show_pilots (line, amount);
}

void    RadioInterface::showIQ  (int amount) {
std::vector<std::complex<float>>Values (amount);
int16_t t;
double  avg     = 0;
int     scopeWidth      = scopeSlider -> value();

	if ((int)iqBuffer. GetRingBufferReadAvailable () < amount)
	   return;
        t = iqBuffer. getDataFromBuffer (Values. data (), amount);
	for (int i = 0; i < t; i ++) {
	   float x = abs (Values [i]);
	   if (!std::isnan (x) && !std::isinf (x))
	   avg += abs (Values [i]);
	}
	avg     /= t;
	my_iqDisplay -> displayIQ (Values, scopeWidth / avg);
}

void    RadioInterface::handle_modeSelector (const QString &m) {
        scopeMode = m == "Pilots" ?  SHOW_PILOTS :
	                  m == "Channel" ? SHOW_CHANNEL : SHOW_ERROR;
}


void	RadioInterface::select_channel_1	() {
	theState. activate_channel_1 ();
}

void	RadioInterface::select_channel_2	() {
	theState. activate_channel_2 ();
}

void	RadioInterface::handle_reset	() {
	fprintf (stderr, "Going to signal\n");
	my_Reader.signal ();
}

void	RadioInterface::handle_presetButton	() {
        if (thePresets. isVisible ())
           thePresets. hide ();
        else
           thePresets. show ();
}

void	RadioInterface::report_vfoFrequency	(int32_t freq) {
	bandLabel	-> setText (my_bandPlan -> getFrequencyLabel (freq));
}

void    RadioInterface::setFrequency   (int32_t freq) {
	if (inputHandler != nullptr) {
	   fprintf (stderr, "set freq to %d (%d)\n", freq, KHz (freq));
           inputHandler -> setVFOFrequency (KHz (freq));
	   bandLabel -> setText (my_bandPlan -> getFrequencyLabel (freq));
	}                
}

void	RadioInterface::handle_saveFreqButton	() {
	if (inputHandler == nullptr) 
	   return;
	if (myLine == NULL)
	   myLine  = new QLineEdit ();
	myLine  -> show ();
	connect (myLine, &QLineEdit::returnPressed,
	         this, &RadioInterface::handle_myLine);
}

void    RadioInterface::handle_myLine () {
	if (inputHandler == nullptr)
	   return;
	QString programName     = myLine -> text ();
	int freq		= inputHandler -> getVFOFrequency ();
	thePresets. addRow (programName, QString::number (freq / 1000));
	disconnect (myLine, &QLineEdit::returnPressed,
	            this,  &RadioInterface::handle_myLine);
	myLine  -> hide ();
	myLine	-> setText ("");
}

//	do not forget that ocnt starts with 1, due
//	to Qt list conventions
void	RadioInterface::setupSoundOut (QComboBox	*streamOutSelector,
	                               audioSink	*our_audioSink,
	                               int32_t		cardRate,
	                               std::vector<int16_t> &table) {
uint16_t	ocnt	= 1;
uint16_t	i;

	for (i = 0; i < our_audioSink -> numberofDevices (); i ++) {
	   const char *so =
	             our_audioSink -> outputChannelwithRate (i, cardRate);

	   if (so != NULL) {
	      streamOutSelector -> insertItem (ocnt, so, QVariant (i));
	      table [ocnt] = i;
	      ocnt ++;
	   }
	}

	qDebug () << "added items to combobox";
	if (ocnt == 1) {
	   QMessageBox::warning (nullptr, tr ("sdr"),
                                       tr ("no valid outstreams\n"));

	   throw (22);
	}
}

void	RadioInterface::setStreamOutSelector (int idx) {
int16_t	outputDevice;

	if (idx == 0)
	   return;

	return;
	outputDevice = outTable [idx];
	if (!audioHandler -> isValidDevice (outputDevice)) 
	   return;

	audioHandler	-> stop	();
	if (!audioHandler -> selectDevice (outputDevice)) {
	   QMessageBox::warning (nullptr, tr ("sdr"),
	                               tr ("Selecting  output stream failed\n"));
	   return;
	}

	qWarning () << "selected output device " << idx << outputDevice;
	audioHandler	-> restart ();
}

void	RadioInterface::handle_quit	() {
	running. store (false);
	my_Reader. stop ();
	m_worker        -> join ();
	delete 	m_worker;
	m_worker = nullptr;
	delete	my_eqDisplay;
	my_eqDisplay	= nullptr;
	delete	my_iqDisplay;
	my_iqDisplay	= nullptr;
	if (inputHandler != nullptr)
	   delete	inputHandler;
	inputHandler	= nullptr;
	fprintf (stderr, "we gaan saven\n");
	thePresets. saveTable ();
	thePresets. hide ();
	close ();
}

void	RadioInterface::show_signalPower	(double v) {
	this	-> snrDisplay -> display (v);
}

