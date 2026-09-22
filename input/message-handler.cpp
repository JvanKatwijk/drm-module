#
/*
 *    Copyright (C)   2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm module
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

#include	<QJsonDocument>
#include	<QJsonObject>
#include	"message-handler.h"

 
float   Alpha   = 1.0 / 12000;

#define	DEC_STEP	(CONV_RATE/WORKING_RATE)
static
QString IQstarter       = "{ \"event_type\":\"iq_stream_enable\",\"property\":\"\",\"value\":\"%1\" }";

	messageHandler::messageHandler (RingBuffer<Complex> *b,
	                                int startFrequency):
	                                    theOscillator (DEVICE_RATE),
	                                    theFilter (15, 5500, DEVICE_RATE),
	                                    theDecimator (DEC_STEP,
	                                                  2 * DEC_STEP, 5500,
	                                                  CONV_RATE),
	                                    _I_Buffer (32 * 32768) {
	_O_Buffer		= b;
	vfo_frequency		= startFrequency;
	float denominator	= float (DEVICE_RATE) / DIVIDER;
	float inVal		= float (DEVICE_RATE) / DIVIDER;
	for (int i = 0; i < CONV_RATE / DIVIDER; i ++) {
	   mapTable_int [i]	= int (floor (i * (inVal / denominator)));
	   mapTable_float [i] =
	                      i * (inVal / denominator) - mapTable_int [i];
	}
	convIndex		= 0;
	theSocket		= nullptr;

	dcReal			= 0;
	dcImag			= 0;
	iqFilter_on		= false;
}

	messageHandler::~messageHandler	() {
	if (runMode)
	   iqStreamEnable (false);
	if (theSocket != nullptr)
	   delete theSocket;
}

void	messageHandler::tryConnect (const QString &hostAddress,
	                                              int portNumber) {
	theSocket	= new socketHandler (hostAddress,
	                                     portNumber, &_I_Buffer);
	connect (theSocket, &socketHandler::reportConnect,
	         this, &messageHandler::connection_set);
	connect (theSocket, &socketHandler::reportDisconnect,
	         this, &messageHandler::reportDisconnect);
	theSocket	-> tryConnect ();
}

void	messageHandler::no_connection	() {
	disconnect (theSocket, &socketHandler::reportConnect,
	            this, &messageHandler::connection_set);
	disconnect (theSocket, &socketHandler::reportDisconnect,
	            this, &messageHandler::no_connection);
	emit connection_failed ();
}

void	messageHandler::reportDisconnect	() {
	disconnect (theSocket, &socketHandler::reportConnect,
	            this, &messageHandler::connection_set);
	disconnect (theSocket, &socketHandler::reportDisconnect,
	            this, &messageHandler::reportDisconnect);
	fprintf (stderr, "Reporting a disconnect\n");
//	delete theSocket;
//	theSocket = nullptr;
	emit set_disconnect ();
}

void	messageHandler::connection_set	() {
	disconnect (theSocket, &socketHandler::reportConnect,
	            this, &messageHandler::connection_set);
	connect (theSocket, &socketHandler::binDataAvailable,
                 this, &messageHandler::binDataAvailable);
	connect (theSocket, &socketHandler::dispatchMessage,
                 this, &messageHandler::dispatchMessage);

	setProperty ("device_sample_rate",
	                              QString::number (DEVICE_RATE));
	set_filterBW	(10500);
	setProperty ("device_center_frequency",
	                              QString::number (vfo_frequency));
        setProperty ("device_vfo_frequency",
	                              QString::number (vfo_frequency));
        askProperty ("device_sample_rate");
	setProperty ("audio_mute", "true");
	emit	connection_succeeded ();
}
//
//	setVFOFrequency is used by the drm code
void	messageHandler::setVFOFrequency	(int freq) {
	setProperty ("device_center_frequency", QString::number (freq));
	setProperty ("device_vfo_frequency", QString::number (freq));
	askProperty ("device_center_frequency");
	askProperty ("device_vfo_frequency");
	this -> vfo_frequency	= freq;
}

int	messageHandler::getVFOFrequency	() {
	return this -> vfo_frequency;
}

void    messageHandler::iqStreamEnable  (bool b) {
	if (theSocket == nullptr)
	   return;
        theSocket -> sendMessage (IQstarter. arg (b ? "true" : "false"));
        askProperty ("device_center_frequency");
}

//	Transfer is in segments of 1 msec
//	DEVICE_RATE = 125000
void	messageHandler::binDataAvailable () {
	std::complex<int16_t>  inBuffer [DEVICE_RATE / 1000];
	while (_I_Buffer. GetRingBufferReadAvailable () >=
	                                            DEVICE_RATE / 1000) {
	   _I_Buffer. getDataFromBuffer (inBuffer, DEVICE_RATE / 1000);
	   if (!runMode)	// only deal with data when processing is on
	      continue;

	   for (int i = 0; i < DEVICE_RATE / 1000; i ++) {
	      Complex temp = Complex (real (inBuffer [i]) / (2 * 2048.0),
	                              imag (inBuffer [i]) / (2 * 2048.0));
	      temp	*= conj (theOscillator.
	                        next (vfo_frequency - center_frequency));
//	      dcReal	= compute_avg (dcReal, real (temp), Alpha);
//	      dcImag	= compute_avg (dcImag, imag (temp), Alpha);
//	      temp	= Complex (real (temp) - dcReal,
//	                           imag (temp) - dcImag);
              if (iqFilter_on)
                 temp = the_iqFilter. process (temp);

	      temp	= theFilter. Pass (temp);

	      Complex localBuf [WORKING_RATE / 1000];
	      convBuffer [convIndex ++] = temp;
//
//	resample from 125000 -> 120000 and then decimate to 12000
	      if (convIndex > CONV_SIZE) {
	         int teller = 0;
	         for (int j = 0; j < CONV_RATE / 1000; j ++) {
	            int16_t inpBase     = mapTable_int [j];
                    float   inpRatio    = mapTable_float [j];
	            Complex temp	=
                                     convBuffer [inpBase + 1] * inpRatio +  
                                     convBuffer [inpBase] * (1 - inpRatio);
	            if (theDecimator. process (temp, temp))
	               localBuf [teller ++]	 = temp;
                 }
	         _O_Buffer -> putDataIntoBuffer (localBuf, teller);
	         convBuffer [0] = convBuffer [CONV_SIZE];
	         convIndex = 1;
	      }
	      if (_O_Buffer -> GetRingBufferReadAvailable () >
	                                             WORKING_RATE / 10)
	         emit dataAvailable (WORKING_RATE / 10);
	   }
	}
}

//
//	On start up we inquire for basic values. after that
//	we react upon changes in the values
void	messageHandler::dispatchMessage	(const QString &m) {
bool b;
QJsonObject obj;
QJsonDocument doc = QJsonDocument::fromJson (m. toUtf8 ());

	if (doc. isNull ())
	   return;	// cannot handle
	obj	= doc. object ();

	QString eventType	= obj ["event_type"]. toString ();
	QString property = obj ["property"]. toString ();
//
//	The initial value inquiries
	if (eventType == "get_property_response") {
	   if (property == "device_sample_rate") {
	      QString samplerate = obj ["value"]. toString ();
              double rate       = samplerate. toDouble (&b);
              if (!b)
                 return;
//      we expect DEVICE_RATE and do not process other rates
              if (rate != DEVICE_RATE) {
//	          emit rateError ();
                 return;
              }
              iqStreamEnable (true);
	      runMode	= true;
           }
	   if (property == "device_center_frequency") {
	      QString freqString = obj ["value"]. toString ();
//	      fprintf (stderr, "response centerfreq %s\n",
//	                                      freqString. toLatin1 (). data ());
	      double freq	= freqString. toDouble (&b);
	      if (!b)
	         return;
	      center_frequency	= (int)freq;
//	      fprintf (stderr, "centerfreq changed %d -> %d\n",
//	                                     (int)freq, freq - vfo_frequency);
	   }
	   if (property == "device_vfo_frequency") {
	      QString freqString = obj ["value"]. toString ();
//	      fprintf (stderr, "response vfofreq %s\n",
//	                                      freqString. toLatin1 (). data ());
	      double freq	= freqString. toDouble (&b);
	      if (!b)
	         return;
	      vfo_frequency	= (int)freq;
//	      fprintf (stderr, "vfo changed %d (%d) -> %d\n",
//	                                    (int)freq,
//	                                    center_frequency,
//	                                    center_frequency - vfo_frequency);
	   }
	}
//
//	The mods in values
	if (eventType == "property_changed") {
	   QString property = obj ["property"]. toString ();
	   if (property == "device_vfo_frequency") {
	      QString vfoString = obj ["value"]. toString ();
	      bool b;
	      int vfo	= vfoString. toInt (&b);
	      if (!b)
	         return;
	      vfo_frequency	= vfo;
//	      fprintf (stderr, "change in vfo %d (%d)\n",
//	                                  vfo_frequency, center_frequency);
	      emit frequency_changed (vfo);
	   }
	   if (property == "device_center_frequency") {
	      QString freqString = obj ["value"]. toString ();
//	      fprintf (stderr, "property device_center_freq %s\n",
//	                             freqString. toLatin1 (). data ());
	      bool b;
	      int centerFreq	= freqString. toInt (&b);
	      if (!b)
	         return;
	      center_frequency	= centerFreq;
//	      fprintf (stderr, "device_center_frequency %d (%d)\n",
//	                                             centerFreq, vfo_frequency);
//	      emit frequency_changed (vfo);
	   }
	   if (property == "signal_snr") {
	      QString snrString = obj ["value"]. toString ();
	      QString res;
	      for (int i = 0; i < snrString. size (); i ++)
                  if (snrString. at (i) == QChar (','))
                     res. push_back (QChar ('.'));
                  else
                     res. push_back (snrString. at (i));
	      bool b;
	      double snr = res. toDouble (&b);
	      if (!b)
	         return;
	      emit signalPower (snr);
	   }
	}
}

void	messageHandler::setProperty (const QString prop, const QString val) {
QString message = "{ \"event_type\":\"set_property\",\"property\":\"%1\",\"value\":\"%2\" }";
	theSocket -> sendMessage (message. arg (prop).arg (val));
}

void	messageHandler::askProperty (const QString prop) {
QString  message = "{ \"event_type\":\"get_property\",\"property\":\"%1\" }";
	theSocket -> sendMessage (message. arg (prop));
}

void	messageHandler::set_filterBW	(uint32_t bw) {
	setProperty ("filter_bandwidth", QString::number (bw));
}

void	messageHandler::set_iqSelect	(bool b) {
	iqFilter_on		= b;
}

