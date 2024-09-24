#include "synth.h"
#include "math.h"

/*
 Sintesizer data
 Melody | Instrument | Volume | Pitch | End | Repeat | Duration
 1'b    | 4'b        | 7'b    | 7'b   | 1'b | 1'b    | 11'b

 TODO: Understand better the syntethizer and use more parameters
*/                 

void play_note(u32 note, u32 duration_in_ms, u32 instrument, f32 volume)
{
	u32 volume_int = clamp(volume, 0, 1) * ((1 << 7) -1);
	u32 data =
		((duration_in_ms & (nbits_mask(11))) << 00) |
		((note & nbits_mask(7))              << 13) |
		((volume_int & nbits_mask(7))        << 20) |
		((instrument & nbits_mask(4))        << 27);

	SINV = data;
	// TODO: This is really necessary ?
	// Wait until sintetizer clock
	while(SINC == 0) {}
}
