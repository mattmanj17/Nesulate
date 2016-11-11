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

	// BRK/interrupt request handler

	half pIRQHandler()
	{
		return halfAt(0xFFFE);
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
	OP_ADC, // Add with Carry
	OP_AND, // Logical AND
	OP_ASL, // Arithmetic Shift Left
	OP_BCC, // Branch if Carry Clear
	OP_BCS, // Branch if Carry Set
	OP_BEQ, // Branch if Equal
	OP_BIT, // Bit Test
	OP_BMI, // Branch if Minus
	OP_BNE, // Branch if Not Equal
	OP_BPL, // Branch if Positive
	OP_BRK, // Force Interrupt
	OP_BVC, // Branch if Overflow Clear
	OP_BVS, // Branch if Overflow Set
	OP_CLC, // Clear Carry Flag
	OP_CLD, // Clear Decimal Mode
	OP_CLI, // Clear Interrupt Disable
	OP_CLV, // Clear Overflow Flag
	OP_CMP, // Compare
	OP_CPX, // Compare X Register
	OP_CPY, // Compare Y Register
	OP_DEC, // Decrement Memory
	OP_DEX, // Decrement X Register
	OP_DEY, // Decrement Y Register
	OP_EOR, // Exclusive OR
	OP_INC, // Increment Memory
	OP_INX, // Increment X Register
	OP_INY, // Increment Y Register
	OP_JMP, // Jump
	OP_JSR, // Jump to Subroutine
	OP_LDA, // Load Accumulator
	OP_LDX, // Load X Register
	OP_LDY, // Load Y Register
	OP_LSR, // Logical Shift Right
	OP_NOP, // NOP
	OP_ORA, // Logical Inclusive OR
	OP_PHA, // Push Accumulator
	OP_PHP, // Push Processor Status
	OP_PLA, // Pull Accumulator
	OP_PLP, // Pull Processor Status
	OP_ROL, // Rotate Left
	OP_ROR, // Rotate Right
	OP_RTI, // Return from Interrupt
	OP_RTS, // Return from Subroutine
	OP_SBC, // Subtract with Carry
	OP_SEC, // Set Carry Flag
	OP_SED, // Set Decimal Flag
	OP_SEI, // Set Interrupt Disable
	OP_STA, // Store Accumulator
	OP_STX, // Store X Register
	OP_STY, // Store Y Register
	OP_TAX, // Transfer Accumulator to X
	OP_TAY, // Transfer Accumulator to Y
	OP_TSX, // Transfer Stack Pointer to X
	OP_TXA, // Transfer X to Accumulator
	OP_TXS, // Transfer X to Stack Pointer
	OP_TYA, // Transfer Y to Accumulator

	OP_INVALID,
};

// http://www.obelisk.me.uk/6502/addressing.html

enum AddresingMode : byte
{
	AM_Imp,		// Implicit
	AM_Acc,		// Accumulator
	AM_Imm,		// Immediate
	AM_ZP,		// Zero Page
	AM_ZPX,		// Zero Page, X
	AM_ZPY,		// Zero Page, Y
	AM_Rel,		// Relative
	AM_Abs,		// Absolute
	AM_AbsX,	// Absolute, X
	AM_AbsY,	// Absolute, Y
	AM_Ind,		// Indirect
	AM_IndX,	// Indirect, X
	AM_IndY		// Indirect, Y
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
	/*8x*/INST_INVALID_______, { OP_STA, AM_IndX }, INST_INVALID_______, INST_INVALID_______, { OP_STY, AM_ZP   }, { OP_STA, AM_ZP   }, { OP_STX, AM_ZP   }, INST_INVALID_______, { OP_DEY, AM_Imp  }, INST_INVALID_______, { OP_TXA, AM_Imp  }, INST_INVALID_______, { OP_STY, AM_Abs  }, { OP_STA, AM_Abs  }, { OP_STX, AM_Abs  }, INST_INVALID_______,
	/*9x*/{ OP_BCC, AM_Rel  }, { OP_STY, AM_IndY }, INST_INVALID_______, INST_INVALID_______, { OP_STY, AM_ZPX  }, { OP_STA, AM_ZPX  }, { OP_STX, AM_ZPY  }, INST_INVALID_______, { OP_TYA, AM_Imp  }, { OP_STA, AM_AbsY }, { OP_TXS, AM_Imp  }, INST_INVALID_______, INST_INVALID_______, { OP_STA, AM_AbsX }, INST_INVALID_______, INST_INVALID_______,
	/*Ax*/{ OP_LDY, AM_Imm  }, { OP_LDA, AM_IndX }, { OP_LDX, AM_Imm  }, INST_INVALID_______, { OP_LDY, AM_ZP   }, { OP_LDA, AM_ZP   }, { OP_LDX, AM_ZP   }, INST_INVALID_______, { OP_TAY, AM_Imp  }, { OP_LDA, AM_Imm  }, { OP_TAX, AM_Imp  }, INST_INVALID_______, { OP_LDY, AM_Abs  }, { OP_LDA, AM_Abs  }, { OP_LDX, AM_Abs  }, INST_INVALID_______,
	/*Bx*/{ OP_BCS, AM_Rel  }, { OP_LDA, AM_IndY }, INST_INVALID_______, INST_INVALID_______, { OP_LDY, AM_ZPX  }, { OP_LDA, AM_ZPX  }, { OP_LDX, AM_ZPY  }, INST_INVALID_______, { OP_CLV, AM_Imp  }, { OP_LDA, AM_AbsY }, { OP_TSX, AM_Imp  }, INST_INVALID_______, { OP_LDY, AM_AbsX }, { OP_LDA, AM_AbsX }, { OP_LDX, AM_AbsY }, INST_INVALID_______,
	/*Cx*/{ OP_CPY, AM_Imm  }, { OP_CMP, AM_IndX }, INST_INVALID_______, INST_INVALID_______, { OP_CPY, AM_ZP   }, { OP_CMP, AM_ZP   }, { OP_DEC, AM_ZP   }, INST_INVALID_______, { OP_INY, AM_Imp  }, { OP_CMP, AM_Imm  }, { OP_DEX, AM_Imp  }, INST_INVALID_______, { OP_CPY, AM_Abs  }, { OP_CMP, AM_Abs  }, { OP_DEC, AM_Abs  }, INST_INVALID_______,
	/*Dx*/{ OP_BNE, AM_Rel  }, { OP_CMP, AM_IndY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_CMP, AM_ZPX  }, { OP_DEC, AM_ZPX  }, INST_INVALID_______, { OP_CLD, AM_Imp  }, { OP_CMP, AM_AbsY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_CMP, AM_AbsX }, { OP_DEC, AM_AbsX }, INST_INVALID_______,
	/*Ex*/{ OP_CPX, AM_Imm  }, { OP_SBC, AM_IndX }, INST_INVALID_______, INST_INVALID_______, { OP_CPX, AM_ZP   }, { OP_SBC, AM_ZP   }, { OP_INC, AM_ZP   }, INST_INVALID_______, { OP_INX, AM_Imp  }, { OP_SBC, AM_Imm  }, { OP_NOP, AM_Imp  }, INST_INVALID_______, { OP_CPX, AM_Abs  }, { OP_SBC, AM_Abs  }, { OP_INC, AM_Abs  }, INST_INVALID_______,
	/*Fx*/{ OP_BEQ, AM_Rel  }, { OP_SBC, AM_IndY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_SBC, AM_ZPX  }, { OP_INC, AM_ZPX  }, INST_INVALID_______, { OP_SED, AM_Imp  }, { OP_SBC, AM_AbsY }, INST_INVALID_______, INST_INVALID_______, INST_INVALID_______, { OP_SBC, AM_AbsX }, { OP_INC, AM_AbsX }, INST_INVALID_______,
};

#undef INST_INVALID

const IntructionInfo InstIFromByte(byte instruction)
{
	return aryInstI[instruction];
}