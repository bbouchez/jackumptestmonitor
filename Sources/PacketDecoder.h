/***************************************************************
 * Name:      PacketDecoder.h
 * Purpose:   Decodes MIDI packet from JACK
 * Author:    Benoit BOUCHEZ (info.at.imodular-synth.com)
 * Created:   2020-03-01
 * Copyright: Benoit BOUCHEZ (imodular-synth.com)
 * License:
 **************************************************************/

#ifndef __PACKET_DECODER_H__
#define __PACKET_DECODER_H__

#include <jack/jack.h>
#include <jack/midiport.h>

void DecodeJACKMIDIPacket (unsigned int PacketTime, jack_midi_data_t* MIDIPacket, unsigned int PacketSize, char* DecodedString);

#endif
