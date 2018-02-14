/*
This file is part of the Epiphany BSP library.

Copyright (C) 2014-2015 Buurlage Wits
Support e-mail: <info@buurlagewits.nl>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
and the GNU Lesser General Public License along with this program,
see the files COPYING and COPYING.LESSER. If not, see
<http://www.gnu.org/licenses/>.
*/

/*
Resets timer and returns old value, in clockcycles starting from 0
unsigned int get_raw_time()
*/

.file    "get_raw_time.s";

.section .text;
.type    get_raw_time, %function;
.global  get_raw_time;

.balign 4;
get_raw_time:

    // Set some constants that are needed

    mov   r1, %low(#-1);            // UINT_MAX
    movt  r1, %high(#-1);           // UINT_MAX
    mov   r2, %low(0xffffff0f);     // mask for the config register
    movt  r2, %high(0xffffff0f);    // mask for the config register
    mov   r3, 0x0010;               // config bit that specifies timer counting cpu ticks

    // Get the current timer value and reset it to max

    movfs r0, ctimer0;              // r0 = timer
    movts ctimer0, r1;              // timer = UINT_MAX; note that this stops the timer
    eor   r0, r0, r1;               // r0 = UINT_MAX - r0; implemented by an xor
    
    // Start the timer again, because setting it to max stops it

    movfs r16, config;              // get current config
    and   r16, r16, r2;             // turn off the old timer bits
    orr   r16, r16, r3;             // turn on the timer bit
    movts config, r16;              // set the config

    rts;                            // return r0

.size    get_raw_time, .-get_raw_time;
