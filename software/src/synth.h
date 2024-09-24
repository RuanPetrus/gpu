#include "defs.h"

#ifndef SYNTH_H
#define SYNTH_H
// Sintetizer address
#define  SINV  (*(volatile u32 *)0xFF200178)
#define  SINC  (*(volatile u32 *)0xFF20017C)

/*
 Sintesizer data
 Melody | Instrument | Volume | Pitch | End | Repeat | Duration
 1'b    | 4'b        | 7'b    | 7'b   | 1'b | 1'b    | 11'b

 TODO: Understand better the syntethizer and use more parameters
*/                 
void play_note(u32 note, u32 duration_in_ms, u32 instrument, f32 volume);

#endif // SYNTH_H
