----------------------------------------------------------------------------
	A DRM DECODER MODULE FOR SDRCONNECT
----------------------------------------------------------------------------

**DRM (Digital Radio Mondiale)** is - as the name suggests - a form of digital
radio. 

In Europe, DRM is not very popular, most stations that have started a DRM 
transmission in the first decade of the century already have stopped. However, 
Romania still has a number of regular DRM transmissions that I can receive, 
the website http://www.hfcc.org/drm/ gives an overview.

![overview](/res/drm-module.png?raw=true)

The DRM receiver is an experimental tool, a heavily reduced swradio, just a 
single decoder for DRM. The decoder is a copy of the decoder in the 
SW receiver software.

The decoder uses the FDK_AAC library for AAC and xHE-AAC decoding.
-------------------------------------------------------------------------
		The GUI
-------------------------------------------------------------------------

![overview](/res/drm-module-2.png?raw=true)

The drm module is a separate program, communicating with sdrconnect using websockets. Assuming sdrConnect is running, touching the **connect** button tells the software to tru to connect.

**Note that the connection assumed a transfer of a 2 M samplerate**

If connection succeeds, the top line will indicate **connected**, to the right of the port number.

Selectinga frequency is - in any case the first time - done using sdrconnect.
The button **save freq** - whem touched will ask for a name and store the combination (name, frequency).
The button **presets** - when touched -- shows a list of saved frequencies.

![overview](/res/drm-module-3.png?raw=true)

A DRM signal is sampled with 12000 Samples/second. A sequence of samples forms a word, and a group of words us used to encode - and therefore to decode - the
relevant data.

If **time syncing** succeeds, the **time sync** label colors green.

To aid in synchronization, the incoming samplestream contains "markers", pilot symbols that help the decoding software in synchronizing ans build an exact copy of the transmitted data.
The incoming data contains markers, **Pilots**, elements with predefined amplitude and phase, helping the software in rebuilding the original date.

The scope left shows the pilots as they are received. Not that the amplitudes of all pilots is - on transmission - the same.

---------------------------------------------------------------------------
Time and freq sync
---------------------------------------------------------------------------

If the software (assumes) syncing in time and frequency succeeds, the
label **time sync** colors green.

On the left side of the GUI ine seens 3 indicators
 * the SNR of the signal as measured by sdrconnect
 * the course offet, telling the offset of the "null" bin in the frequency somain related to its nominal position;
 * the fine Offset, telling the offset within the correct bin. Note that the bin width is app 43 Hz, so the fine offset is usually a few Hz.

-------------------------------------------------------------------------
FAC data
--------------------------------------------------------------------------

Next to some data supporting the synchronization, the incoming data contains so-called FAC data, **Fast Access Channel**.
This data contains some general information on the transmission and the build up of the data stream.

The GUI contains a label **FAC sync** that colors green if the FAC data can be decoded and passes a CRC test.

------------------------------------------------------------------------------
SDC data
-----------------------------------------------------------------------------

The Service(s) are described in the SDC data part,
**Service Description Channel**.
The SDC data contains information to extract and decode the actual **payload**.

The GUI contains a label **SDC sync** that colors green if the SDC data can
be decoded and passes a CRC test.

-----------------------------------------------------------------------------
AAC data
----------------------------------------------------------------------------

The actual payload consist of up to 4 services, audio or data.
The Audio data is encoded as AAC or xHe-AAC. If extracting a interpreting of the audio data succeeds, the label **aac Sync** colors green.

---------------------------------------------------------------------------
Content description
---------------------------------------------------------------------------

The middle line in the GUI shows at the left hand side some numbers, and at the
right hand side some descriptive data.

The numbers indicate quality indicators of resp. the FAC signals and their decosing, the SDC sigbals and their decoding and the audi decosing.
In general, higher is better.

The labels on the right hand side tell from left to right:
 * the spectrum occupancy of the signal, 3 - the most commonly used - tells a spectrumwidth of 9 kH;
 * the Mode of the encoding, here mode B. The modes differ in the density and positioning of the pilots in the signal;
 * the mode of data encoding, here QAM16 tells that per sample 16 positions can be distinguisged, i.e. 4 bits.
 & the audio encoding, here AAC

--------------------------------------------------------------------------
The IQ display
-------------------------------------------------------------------------

The scope on the botton right, the **IQ-display** shows the position of the decoded values from the service. Ideally one would see - for QAM16 - a clean grid with data centered around the 16 points.

---------------------------------------------------------------------------
Executables
---------------------------------------------------------------------------

For Linux an **appImage** is available, for Windows an **installer**.






Copyright
-------------------------------------------------------------------------------

 Copyright

        Copyright (C)  2019, 2026
        Jan van Katwijk (J.vanKatwijk@gmail.com)
        Lazy Chair Computing 

 	drm module is free software; you can redistribute it and/or modify
 	it under the terms of the GNU General Public License as published by
 	the Free Software Foundation,  version 2 of the License.

 	drm module is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 	GNU General Public License for more details.

 	You should have received a copy of the GNU General Public License
 	along with drm module; if not, write to the Free Software
 	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

