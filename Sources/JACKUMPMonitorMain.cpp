/***************************************************************
 * JACKUMPMonitorMain.cpp
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

#include "JACKUMPMonitorMain.h"
#include "PacketDecoder.h"

#define TIMER_ID			1000

// Global variables
jack_client_t* JackClient=0;
jack_port_t* InputPort=0;
extern CJACKUMPMonFrame* JACKUMPMonFrame;
unsigned int TotalFrames=0;			// Sums frame from application start (to show timestamps)

wxBEGIN_EVENT_TABLE(CJACKUMPMonFrame, wxFrame)
	EVT_TIMER(TIMER_ID, CJACKUMPMonFrame::OnTimer)
wxEND_EVENT_TABLE()

// Callback function called when there is an audio block to process
int jack_process (jack_nframes_t nframes, void* arg)
{
	unsigned int i;
    void* in_port_buf = jack_port_get_buffer(InputPort, nframes);
    jack_midi_event_t in_event;
    jack_nframes_t event_count = jack_midi_get_event_count(in_port_buf);

    if(event_count >= 1)
    {
		for(i=0; i<(unsigned int)event_count; i++)
		{
			jack_midi_event_get(&in_event, in_port_buf, i);
			if (JACKUMPMonFrame)
			{
				JACKUMPMonFrame->StoreJACKEvent(TotalFrames, &in_event);
			}
		}
    }

	TotalFrames+=nframes;

    return 0;
}  // jack_process
// ------------------------------------------------------

/* Callback function called when jack server is shut down */
void jack_shutdown (void* arg)
{
}  // jack_shutdown
// ------------------------------------------------------

CJACKUMPMonFrame::CJACKUMPMonFrame(wxFrame *frame)
    : GUIFrame(frame), m_timer (this, TIMER_ID)
{
	jack_status_t JackStatus;

	FIFOReadPos=0;
	FIFOWritePos=0;

    JackClient = jack_client_open("jackumpmon", JackNullOption, &JackStatus);
    if (JackClient==0)
    {
		// TODO : display value of JackStatus to help finding the error cause
        wxMessageBox("JACK server is not running\nPlease start JACK before launching this application", "Error", wxOK+wxICON_ERROR);
        Destroy();
		return;
    }

    jack_set_process_callback(JackClient, jack_process, 0);
    jack_on_shutdown(JackClient, jack_shutdown, 0);

    InputPort = jack_port_register (JackClient, "Monitor Input", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

    if (jack_activate (JackClient))
    {
        wxMessageBox ("Can not activate JACK client", "Error", wxOK+wxICON_ERROR);
    }

	m_timer.Start (40);
}  // CJACKUMPMonFrame::CJACKUMPMonFrame
// ------------------------------------------------------

CJACKUMPMonFrame::~CJACKUMPMonFrame()
{
    if (JackClient)
    {
		jack_deactivate (JackClient);
        jack_client_close (JackClient);
    }
}  // CJACKUMPMonFrame::~CJACKUMPMonFrame
// ------------------------------------------------------

//! Click on close icon on top bar or request to close window sent by OnQuit
void CJACKUMPMonFrame::OnClose(wxCloseEvent &event)
{
	m_timer.Stop();
    Destroy();
}  // CJACKUMPMonFrame::OnClose
// ------------------------------------------------------

//! Quit command by menu (not the icon on top bar)
void CJACKUMPMonFrame::OnQuit(wxCommandEvent &event)
{
	m_timer.Stop();
    Destroy();
}  // CJACKUMPMonFrame::OnQuit
// ------------------------------------------------------

void CJACKUMPMonFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox(wxString::Format
                 (
                    "JACK UMP Monitor V%d.%d\n\n(c) 2020-2024 Benoit BOUCHEZ", VERSION_MAJOR, VERSION_MINOR
                 ),
                 "About jackumpmonitor",
                 wxOK | wxICON_INFORMATION,
                 this);
}  // CJACKUMPMonFrame::OnAbout
// ------------------------------------------------------

void CJACKUMPMonFrame::OnMenuClear (wxCommandEvent& event)
{
	this->UMPMonLog->Clear();
}  // CJACKUMPMonFrame::OnMenuClear
// ------------------------------------------------------

void CJACKUMPMonFrame::OnTimer(wxTimerEvent& event)
{
	unsigned int TempRead;
	char TraceStr[256];

	// Check if we have something in the FIFO
	if (FIFOReadPos==FIFOWritePos) return;

	TempRead=FIFOReadPos;
	while (TempRead!=FIFOWritePos)		// TODO : maybe use a snapshot for the write position to avoid endless loop if MIDI is flooded
	{
        // Decode data from FIFO
        DecodeJACKMIDIPacket(MIDIFIFO[TempRead].PacketTime, &MIDIFIFO[TempRead].PacketData[0], MIDIFIFO[TempRead].PacketSize, &TraceStr[0]);
        this->UMPMonLog->AppendText(&TraceStr[0]);

		// Limit log memory to 100 lines to avoid overloading CPU with huge wxTextCtrl
        if (this->UMPMonLog->GetNumberOfLines()>=100)
        {
            int RemovePosition=this->UMPMonLog->XYToPosition(0, 1);
            if (RemovePosition>0)
                this->UMPMonLog->Remove(0, RemovePosition);
        }

        // move to next entry in FIFO
		TempRead+=1;
		if (TempRead>=MIDI_FIFO_SIZE)
		{
			TempRead=0;		// Wrap around FIFO
		}
	}
	FIFOReadPos=TempRead;
}  // CJACKUMPMonFrame::OnTimer
// ------------------------------------------------------

void CJACKUMPMonFrame::StoreJACKEvent(unsigned int TotalBase, jack_midi_event_t* JACKEvent)
{
    unsigned int TempWrite=FIFOWritePos;
    size_t TempSize;

    // Store field from event into the current FIFO position
    MIDIFIFO[TempWrite].PacketTime=TotalBase+JACKEvent->time;

    // Clip maximum message size if necessary
    TempSize=JACKEvent->size;
    if (TempSize>MAX_MIDI_PACKET_SIZE)
    {
        // TODO : set a marker to say that message has been clipped
        TempSize=MAX_MIDI_PACKET_SIZE;
    }
    MIDIFIFO[TempWrite].PacketSize=TempSize;

    // Copy data into the FIFO
    memcpy (&MIDIFIFO[TempWrite].PacketData, &JACKEvent->buffer[0], TempSize);

    // Move pointer if possibe
    TempWrite+=1;
    if (TempWrite>=MIDI_FIFO_SIZE) TempWrite=0;
    // Check if no collision with Read Pointer
    if (TempWrite!=FIFOReadPos) FIFOWritePos=TempWrite;
}  // CJACKUMPMonFrame::StoreJACKEvent
// ------------------------------------------------------
