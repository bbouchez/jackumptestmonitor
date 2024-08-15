/***************************************************************
 * JACKUMPMonitorApp.cpp
 * Test application to monitor Universal MIDI Packets from JACK server
 * Main application class
 * Copyright Benoit BOUCHEZ (BEB) - 2020/2024
 *
 * Required libraries for compilation :
 * - JACK
 * - wxWidgets 3.0 or higher
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

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "JACKUMPMonitorApp.h"
#include "JACKUMPMonitorMain.h"

CJACKUMPMonFrame* JACKUMPMonFrame=0;			// Make frame global so JACK callback can access it

IMPLEMENT_APP(JACKUMPMonApp);

bool JACKUMPMonApp::OnInit()
{
    JACKUMPMonFrame = new CJACKUMPMonFrame(0L);
    JACKUMPMonFrame->Show();

    return true;
}
