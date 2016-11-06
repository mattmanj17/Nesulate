#pragma once
#include "types.h"

// see http://nesdev.com/2A03%20technical%20reference.txt

class CPU_2A03
{
public:

private:
	// PINS (EXTERNAL STATE)

	/*        ________
	         |*  \/   |
	ROUT  <01]        [40<  VCC
	COUT  <02]        [39>  $4016W.0
	/RES  >03]        [38>  $4016W.1
	A0    <04]        [37>  $4016W.2
	A1    <05]        [36>  /$4016R
	A2    <06]        [35>  /$4017R
	A3    <07]        [34>  R/W
	A4    <08]        [33<  /NMI
	A5    <09]        [32<  /IRQ
	A6    <10]  2A03  [31>  PHI2
	A7    <11]        [30<  ---
	A8    <12]        [29<  CLK
	A9    <13]        [28]  D0
	A10   <14]        [27]  D1
	A11   <15]        [26]  D2
	A12   <16]        [25]  D3
	A13   <17]        [24]  D4
	A14   <18]        [23]  D5
	A15   <19]        [22]  D6
	VEE   >20]        [21]  D7
	         |________|
	*/

	// ROUT
	// this signal carries the mixed outputs for both internal rectangle wave 
	// function generators
	float sigoROUT;

	// COUT
	// this signal carries the combined outputs for an internal triangle 
	// wave / random wave function generator, and a programmable 7 - bit DAC controlled
	// by a delta counter / DMA timer unit combination
	float sigoCOUT;

	// RES
	// hard reset on falling edge (1->0). Resets the status of several internal 2A03 
	// registers, and the 6502
	void RES();

	// A0-A15
	// he 6502's address bus output pins
	u16 A;

	// VEE, VCC: ground, and + 5VDC power signals, respectfully.
	// not simulated

	// D0-D7
	// the 6502's data bus
	u8 D;

	// CLK 
	// 2A03's master clock
	// 236250/11 KHz
	// clocks an internal divide-by-12 counter
	void CLK();

	// ---
	// grounded in NES console
	// unknown functionality

	// PHI2
	// the divide-by-12 result of the CLK signal (1.79 MHz)
	// The internal 6502 along with function generating hardware, is clocked off 
	// this frequency, and is available externally here so that it can be used as a
	// data bus enable signal(when at logic level 1) for external 6502 address
	// decoder logic.
	void PHI2();

	// IRQ
	// interrupts the 6502
	// - when this pin is on a falling edge (1->0)
	// - while the 6502's internal interrupt mask flag is 0
	void IRQ();

	// NMI
	// Non-maskable interrupt
	// NMI's the 6502 on a falling edge (1->0)
	void NMI();

	// R/W
	// direction of 6502's data bus
	// 0=write, 1=read
	bool RW;

	// 4017R
	// on a PHI2, if we are writing to $4017
	// put controller port data onto data bus
	void PutControlerData();

	// 4016R
	// on a PHI2, if we are writing to $4016
	// write to internal 3-bit register
	// bit 0 is used as a "strobe line" 
	// for the shift register inside the controller
	// see https://en.wikipedia.org/wiki/Data_strobe_encoding
	u8 _4016;
	void Write4016();

	// INTERNAL STATE

	// SOUND HARDWARE

	// MISC HARDWARE

	// 6502 CPU (lacking decimal mode support)
	
	
};
