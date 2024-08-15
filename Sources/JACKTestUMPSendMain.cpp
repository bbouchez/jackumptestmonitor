/***************************************************************
 * JACKTestUMPSendMain.cpp
 * Test application to send Universal MIDI Packets through JACK server
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

#include "JACKTestUMPSendMain.h"

// Global variables
jack_client_t* JackClient=0;
jack_port_t* OutputPort=0;
bool DoTest1=false;
bool DoTest2=false;
bool DoTest3=false;
bool DoTest4=false;

wxBEGIN_EVENT_TABLE(CJACKUMPSenderFrame, wxFrame)
	//EVT_TIMER(TIMER_ID, CJACKMIDIMonFrame::OnTimer)
wxEND_EVENT_TABLE()

// Callback function called when there is an audio block to process
int jack_process (jack_nframes_t nframes, void* arg)
{
	void* out_port_buf = jack_port_get_buffer(OutputPort, nframes);
	jack_midi_data_t* Buffer;

	jack_midi_clear_buffer(out_port_buf);    // Recommended to call this at the beginning of process cycle

	if (DoTest1)
	{	// Test MIDI 1.0
		Buffer=jack_midi_event_reserve (out_port_buf, 0, 3);
		if (Buffer!=0)
		{
			Buffer[0]=0xB0;
			Buffer[1]=0x40;
			Buffer[2]=0x7F;
		}
		DoTest1=false;
	}

	// Test sending 32 bits UMP message
	if (DoTest2)
	{
		Buffer=jack_midi_event_reserve (out_port_buf, 0, 4);
		if (Buffer!=0)
		{
			Buffer[0]=0x20;     // MT=2, group=0
			Buffer[1]=0xB0;
			Buffer[2]=0x40;
			Buffer[3]=0x7F;
		}
		DoTest2=false;
	}

	// Test sending 64 bits UMP message
	if (DoTest3)
	{
		Buffer=jack_midi_event_reserve (out_port_buf, 0, 8);
		if (Buffer!=0)
		{
			Buffer[0]=0x40;     // MT=4, group=0
			Buffer[1]=0xB0;
			Buffer[2]=0x40;
			Buffer[3]=0x00;
			Buffer[4]=0x12;     // 32 bits CC
			Buffer[5]=0x34;
			Buffer[6]=0x56;
			Buffer[7]=0x78;
		}
		DoTest3=false;
	}

	// Test sending 128 bits UMP message (Device Identity Notification)
	if (DoTest4)
	{
		Buffer=jack_midi_event_reserve (out_port_buf, 0, 16);
		if (Buffer!=0)
		{
			Buffer[0]=0xF0;     // MT=F
			Buffer[1]=0x02;     // Status=2
			Buffer[2]=0x00;
			Buffer[3]=0x00;

			Buffer[4]=0x00;
			Buffer[5]=0x00;     // SYSEX ID 1
			Buffer[6]=0x20;     // SYSEX ID 2
			Buffer[7]=0x7C;     // SYSEX ID 3  (thank you KissBox BV!!! °-) )

			Buffer[8]=0x01;     // Device family LSB
			Buffer[9]=0x00;     // Device family MSB
			Buffer[10]=0x01;    // Device model LSB
			Buffer[11]=0x00;    // Device model MSB

			Buffer[12]=0x01;    // SW revision = 1.0.0.0
			Buffer[13]=0x00;
			Buffer[14]=0x00;
			Buffer[15]=0x00;
		}

        // Send immediately another UMP message to test consecutive UMP messages
        /*
		Buffer=jack_midi_event_reserve (out_port_buf, 0, 4);
		if (Buffer!=0)
		{
			Buffer[0]=0x20;     // MT=2, group=0
			Buffer[1]=0xB0;
			Buffer[2]=0x40;
			Buffer[3]=0x7F;
		}
		*/

		DoTest4=false;
	}

    return 0;
}  // jack_process
// ------------------------------------------------------

/* Callback function called when jack server is shut down */
void jack_shutdown (void* arg)
{
}  // jack_shutdown
// ------------------------------------------------------

CJACKUMPSenderFrame::CJACKUMPSenderFrame(wxFrame *frame) : GUIFrame(frame)
{
	jack_status_t JackStatus;

    JackClient = jack_client_open("jackumpsender", JackNullOption, &JackStatus);
    if (JackClient==0)
    {
		// TODO : display value of JackStatus to help finding the error cause
        wxMessageBox("JACK server is not running\nPlease start JACK before launching this application", "Error", wxOK+wxICON_ERROR);
        Destroy();
		return;
    }

    jack_set_process_callback(JackClient, jack_process, 0);
    jack_on_shutdown(JackClient, jack_shutdown, 0);

    OutputPort = jack_port_register (JackClient, "Tester Output", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

    if (jack_activate (JackClient))
    {
        wxMessageBox ("Can not activate JACK client", "Error", wxOK+wxICON_ERROR);
    }
}  // CJACKUMPSenderFrame::CJACKUMPSenderFrame
// ------------------------------------------------------

CJACKUMPSenderFrame::~CJACKUMPSenderFrame()
{
    if (JackClient)
    {
		jack_deactivate (JackClient);
        jack_client_close (JackClient);
    }
}  // CJACKUMPSenderFrame::~CJACKUMPSenderFrame
// ------------------------------------------------------

//! Click on close icon on top bar or request to close window sent by OnQuit
void CJACKUMPSenderFrame::OnClose(wxCloseEvent &event)
{
    this->Destroy();
}  // CJACKUMPSenderFrame::OnClose
// ------------------------------------------------------

//! Quit command by menu (not the icon on top bar)
void CJACKUMPSenderFrame::OnQuit(wxCommandEvent &event)
{
    this->Close(true);
}  // CJACKUMPSenderFrame::OnQuit
// ------------------------------------------------------

void CJACKUMPSenderFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox(wxString::Format
                 (
                    "JACK UMP Sender Test V%d.%d\n\n(c) 2020/2024 Benoit BOUCHEZ", VERSION_MAJOR, VERSION_MINOR
                 ),
                 "About jacktestumpsend",
                 wxOK | wxICON_INFORMATION,
                 this);
}  // CJACKUMPSenderFrame::OnAbout
// ------------------------------------------------------

void CJACKUMPSenderFrame::OnBtnTest1Click( wxCommandEvent& event )
{
	DoTest1=true;
}  // CJACKUMPSenderFrame::OnBtnTest1Click
// ------------------------------------------------------

void CJACKUMPSenderFrame::OnBtnTest2Click( wxCommandEvent& event )
{
	DoTest2=true;
}  // CJACKUMPSenderFrame::OnBtnTest2Click
// ------------------------------------------------------

void CJACKUMPSenderFrame::OnBtnTest3Click( wxCommandEvent& event )
{
	DoTest3=true;
}  // CJACKUMPSenderFrame::OnBtnTest3Click
// ------------------------------------------------------

void CJACKUMPSenderFrame::OnBtnTest4Click( wxCommandEvent& event )
{
	DoTest4=true;
}  // CJACKUMPSenderFrame::OnBtnTest4Click
// ------------------------------------------------------
