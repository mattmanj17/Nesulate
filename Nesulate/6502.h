#include "Types.h"


//http://www.obelisk.me.uk/6502/index.html

class CPU_6502
{
public:

private:

	// capable of addressing at most 64Kb of memory via 16 bit address bus
	
	byte ram[64 * KB];

	// The first 256 byte page of memory ($0000-$00FF) is referred to as 
	// 'Zero Page' and is the focus of a number of special addressing modes

	// The second page of memory ($0100-$01FF) is reserved for the 
	// system stack, which cannot be relocated.

	// reserved locations in the memory map are the very last 6 bytes of memory $FFFA to $FFFF 
	// which must be programmed with the addresses of the non-maskable interrupt handler ($FFFA/B), 
	// the power on reset location ($FFFC/D) and the BRK/interrupt request handler ($FFFE/F) respectively.
	
	half halfAt(half addr)
	{
		// both The 6502 and x86 are little endian, so this works 
		
		return *((half*)(ram + addr));
	}

	// non-maskable interrupt handler

	half pNMIHandler()
	{
		return halfAt(0xFFFA);
	}

	// power on reset location

	half pReset()
	{
		return halfAt(0xFFFC);
	}

	// registers
	
	half pc;
	half sp = 0x01FF; // intitaly points to begining of stack. decremented on push, incremented on pop
	byte acc;
	byte iX;
	byte iY;

	// status flags

	/*
		when pushed to stack
		
		7  bit  0
		---- ----
		NV1s DIZC
		|||| ||||
		|||| |||+- Carry: 1 if last addition or shift resulted in a carry, or if
		|||| |||     last subtraction resulted in no borrow
		|||| ||+-- Zero: 1 if last operation resulted in a 0 value
		|||| |+--- Interrupt: Interrupt inhibit
		|||| |       (0: /IRQ and /NMI get through; 1: only /NMI gets through)
		|||| +---- Decimal: 1 to make ADC and SBC use binary-coded decimal arithmetic
		||||         (ignored on second-source 6502 like that in the NES)
		|||+------ s: push source. 1 if pushed by instruction, 0 if pushed by interupt. not viewable when not pushed
		||+------- always 1, not viewable when not pushed
		|+-------- Overflow: 1 if last ADC or SBC resulted in signed overflow,
		|            or D6 from last BIT
		+--------- Negative: Set to bit 7 of the last operation
	*/

	enum StatusFlags : byte
	{
		StatusFlag_Carry			= 1 << 0,
		StatusFlag_Zero				= 1 << 1,
		StatusFlag_InteruptDisable	= 1 << 2,
		StatusFlag_Decimal			= 1 << 3,
		StatusFlag_PushSource		= 1 << 4,
		StatusFlag_AlwaysOne		= 1 << 5,
		StatusFlag_Overflow			= 1 << 6,
		StatusFlag_Negative			= 1 << 7,
	};

	byte status;
};

// http://www.obelisk.me.uk/6502/reference.html

enum OpCode : byte
{
	OP_ADC,
	OP_AND,
	OP_ASL,
	OP_BBC,
	OP_BCS,
	OP_BEQ,
	OP_BIT,
	OP_BMI,
	OP_BNE,
	OP_BPL,
	OP_BRK,
	OP_BVC,
	OP_BVS,
	OP_BVS,
	OP_CLC,
	OP_CLD,
	OP_CLI,
	OP_CLV,
	OP_CMP,
	OP_CPX,
	OP_CPY,
	OP_DEC,
	OP_DEX,
	OP_DEY,
	OP_EOR,
	OP_INC,
	OP_INX,
	OP_INY,
	OP_JMP,
	OP_JSR,
	OP_LDA,
	OP_LDX,
	OP_LDY,
	OP_LSR,
	OP_NOP,
	OP_ORA,
	OP_PHA,
	OP_PHP,
	OP_PLA,
	OP_PLP,
	OP_ROL,
	OP_ROR,
	OP_RTI,
	OP_RTS,
	OP_SBC,
	OP_SEC,
	OP_SED,
	OP_SEI,
	OP_STA,
	OP_STX,
	OP_STY,
	OP_TAX,
	OP_TAY,
	OP_TSX,
	OP_TXA,
	OP_TXS,
	OP_TYA,

	OP_INVALID,
};

// http://www.obelisk.me.uk/6502/addressing.html

enum AddresingMode : byte
{
	AM_Imp,
	AM_Acc,
	AM_Imm,
	AM_ZP,
	AM_ZPX,
	AM_ZPY,
	AM_Rel,
	AM_Abs,
	AM_AbsX,
	AM_AbsY,
	AM_Ind,
	AM_IndX,
	AM_IndY
};

struct IntructionInfo
{
	OpCode op;
	AddresingMode am;
};

#define INST_INVALID_______ {OP_INVALID, AM_Imp}

// lookup table that converts from byte to opcode and addressing mode

const IntructionInfo aryInstI[256] =
{
	// http://www.llx.com/~nparker/a2/opcodes.html
	// http://www.obelisk.me.uk/6502/reference.html
	// https://en.wikipedia.org/wiki/MOS_Technology_6502#Assembly_language_instructions
	/*  | x0                 | x1                 | x2                 | x3                 | x4                 | x5                 | x6                 | x7                 | x8                 | x9                 | xA                 | xB                 | xC                 | xD                 | xE                 | xF                 |*/
	/*0x*/{ OP_BRK, AM_Imp  }, { OP_ORA, AM_IndX }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_ORA, AM_ZP   }, { OP_ASL, AM_ZP   }, INST_INVALID_______, { OP_PHP, AM_Imp  }, { OP_ORA, AM_Imm  }, { OP_ASL, AM_Acc  }, INST_INVALID_______, INST_INVALID_______, { OP_ORA, AM_Abs  }, { OP_ASL, AM_Abs  }, INST_INVALID_______,
	/*1x*/{ OP_BPL, AM_Rel  }, { OP_ORA, AM_IndY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_ORA, AM_ZPX  }, { OP_ASL, AM_ZPX  }, INST_INVALID_______, { OP_CLC, AM_Imp  }, { OP_ORA, AM_AbsY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_ORA, AM_AbsX }, { OP_ASL, AM_AbsX }, INST_INVALID_______,
	/*2x*/{ OP_JSR, AM_Abs  }, { OP_AND, AM_IndX }, INST_INVALID_______, INST_INVALID_______, { OP_BIT, AM_ZP   }, { OP_AND, AM_ZP   }, { OP_ROL, AM_ZP   }, INST_INVALID_______, { OP_PLP, AM_Imp  }, { OP_AND, AM_Imm  }, { OP_ROL, AM_Acc  }, INST_INVALID_______, { OP_BIT, AM_Abs  }, { OP_AND, AM_Abs  }, { OP_ROL, AM_Abs  }, INST_INVALID_______,
	/*3x*/{ OP_BMI, AM_Rel  }, { OP_AND, AM_IndY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_AND, AM_ZPX  }, { OP_ROL, AM_ZPX  }, INST_INVALID_______, { OP_SEC, AM_Imp  }, { OP_AND, AM_AbsY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_AND, AM_AbsX }, { OP_ROL, AM_AbsX }, INST_INVALID_______,
	/*4x*/{ OP_RTI, AM_Imp  }, { OP_EOR, AM_IndX }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_EOR, AM_ZP   }, { OP_LSR, AM_ZP   }, INST_INVALID_______, { OP_PHA, AM_Imp  }, { OP_EOR, AM_Imm  }, { OP_LSR, AM_Acc  }, INST_INVALID_______, { OP_JMP, AM_Abs  }, { OP_EOR, AM_Abs  }, { OP_LSR, AM_Abs  }, INST_INVALID_______,
	/*5x*/{ OP_BVC, AM_Rel  }, { OP_EOR, AM_IndY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_EOR, AM_ZPX  }, { OP_LSR, AM_ZPX  }, INST_INVALID_______, { OP_CLI, AM_Imp  }, { OP_EOR, AM_AbsY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_EOR, AM_AbsX }, { OP_LSR, AM_AbsX }, INST_INVALID_______,
	/*6x*/{ OP_RTS, AM_Imp  }, { OP_ADC, AM_IndX }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_ADC, AM_ZP   }, { OP_ROR, AM_ZP   }, INST_INVALID_______, { OP_PLA, AM_Imp  }, { OP_ADC, AM_Imm  }, { OP_ROR, AM_Acc  }, INST_INVALID_______, { OP_JMP, AM_Ind  }, { OP_ADC, AM_Abs  }, { OP_ROR, AM_Abs  }, INST_INVALID_______,
	/*7x*/{ OP_BVS, AM_Rel  }, { OP_ADC, AM_IndY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_ADC, AM_ZPX  }, { OP_ROR, AM_ZPX  }, INST_INVALID_______, { OP_SEI, AM_Imp  }, { OP_ADC, AM_AbsY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_ADC, AM_AbsX }, { OP_ROR, AM_AbsX }, INST_INVALID_______,
	/*8x*/
	/*9x*/
	/*Ax*/
	/*Bx*/
	/*Cx*/
	/*Dx*/
	/*Ex*/
	/*Fx*/
};

#undef INST_INVALID

const IntructionInfo * InstIFromByte(byte instruction)
{
	return &(aryInstI[instruction]);
}