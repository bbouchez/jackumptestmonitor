/***************************************************************
 * JACKUMPMonitorMain.h
 * Test application to monitor Universal MIDI Packets from JACK server
 * Main window class
 * Copyright Benoit BOUCHEZ (BEB) - 2020/2024
 *
 * License : MIT
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **************************************************************/

#ifndef __JACKUMPMONMAIN_H__
#define __JACKUMPMONMAIN_H__

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <jack/jack.h>
#include <jack/midiport.h>

#include "GUIFrame.h"

#define VERSION_MAJOR	0
#define VERSION_MINOR	1

//! Maximum number of bytes in a MIDI message
#define MAX_MIDI_PACKET_SIZE	128
//! Number of entries in the MIDI FIFO
#define MIDI_FIFO_SIZE		256

typedef struct {
	jack_nframes_t PacketTime;
	size_t PacketSize;
	jack_midi_data_t PacketData[MAX_MIDI_PACKET_SIZE];
} TMIDIFIFO_ENTRY;

class CJACKUMPMonFrame: public GUIFrame
{
    public:
        CJACKUMPMonFrame(wxFrame *frame);
        ~CJACKUMPMonFrame();

		void StoreJACKEvent(unsigned int TotalBase, jack_midi_event_t* JACKEvent);
    private:
		wxDECLARE_EVENT_TABLE();

        virtual void OnClose(wxCloseEvent& event);
        virtual void OnQuit(wxCommandEvent& event);
        virtual void OnAbout(wxCommandEvent& event);
		virtual void OnMenuClear (wxCommandEvent& event);

		void OnTimer(wxTimerEvent& event);

		wxTimer m_timer;

		// MIDI FIFO from Jack to display
		TMIDIFIFO_ENTRY MIDIFIFO[MIDI_FIFO_SIZE];
		unsigned int FIFOReadPos;
		unsigned int FIFOWritePos;
};

#endif
