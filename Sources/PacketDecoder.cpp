/***************************************************************
 * Name:      PacketDecoder.cpp
 * Purpose:   Decodes MIDI packet from JACK
 * Author:    Benoit BOUCHEZ (info.at.imodular-synth.com)
 * Created:   2020-03-01
 * Copyright: Benoit BOUCHEZ (imodular-synth.com)
 * License:
 **************************************************************/

 #include "PacketDecoder.h"
 #include <stdio.h>
 #include <cstring>
 #include <arpa/inet.h>

void DecodeJACKMIDIPacket (unsigned int PacketTime, jack_midi_data_t* MIDIPacket, unsigned int PacketSize, char* DecodedString)
{
    unsigned int ByteCount;
    char TmpStr[256];
	unsigned int UMPWord;

    // if length is 1 to 3 bytes, we are sure this is a MIDI 1.0 message
    if ((PacketSize>=1)&&(PacketSize<=3))
    {
        sprintf (DecodedString, "%d MIDI 1.0 - ", PacketTime);
        for (ByteCount=0; ByteCount<PacketSize; ByteCount++)
        {
            sprintf (&TmpStr[0], "%X ", MIDIPacket[ByteCount]);
            strcat (DecodedString, &TmpStr[0]);
        }
        strcat (DecodedString, "\n");
        return;
    }

    // if length is 4, 8 or 16 bytes (32, 64 or 128 bits) and not starting by 0xF0, we are sure this is a UMP message
    // Special case : message starting with 0xF0 and length of 16 bytes are UMP of type MT=0x0F / group=0.
    if ((PacketSize==4) || (PacketSize==8) || (PacketSize==16))
    {
		// Add always the first 32-bit word
		memcpy (&UMPWord, &MIDIPacket[0], 4);
		UMPWord=htonl(UMPWord);
        sprintf (DecodedString, "%d MIDI 2.0 - %08X", PacketTime, UMPWord);

		// if size = 8 or 16, decode the second word
		if (PacketSize>4)
		{
			memcpy (&UMPWord, &MIDIPacket[4], 4);
            UMPWord=htonl(UMPWord);
			sprintf (&TmpStr[0], " %08X", UMPWord);
			strcat (DecodedString, &TmpStr[0]);
		}

		// if size = 16, display third and fourth word
		if (PacketSize==16)
		{
			memcpy (&UMPWord, &MIDIPacket[8], 4);
            UMPWord=htonl(UMPWord);
			sprintf (&TmpStr[0], " %08X", UMPWord);
			strcat (DecodedString, &TmpStr[0]);

			memcpy (&UMPWord, &MIDIPacket[12], 4);
            UMPWord=htonl(UMPWord);
			sprintf (&TmpStr[0], " %08X", UMPWord);
			strcat (DecodedString, &TmpStr[0]);
		}

		strcat (DecodedString, "\n");
        return;
    }

    // In all other cases, this is a MIDI 1.0 SYSEX message
    sprintf (DecodedString, "%d MIDI 1.0 SYSEX - Size %d\n", PacketTime, PacketSize);
}  // DecodeJACKMIDIPacket
// ------------------------------------------------------
