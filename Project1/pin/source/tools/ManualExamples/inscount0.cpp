/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

ofstream OutFile;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;

// This function is called before every instruction is executed
VOID printStatus(VOID* ip,
		ADDRINT* regEAX,
		ADDRINT* regEBX,
		ADDRINT* regECX,
		ADDRINT* regEDX,
		ADDRINT* regEDI,
		ADDRINT* regESI,
		ADDRINT* regR8B) 
{ 
    printf("%lx: %lx %lx %lx %lx %lx %lx %lx \n", (ADDRINT)ip,
	*regEAX, *regEBX, *regECX, *regEDX, *regEDI, *regESI, *regR8B);
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
     ADDRINT addr = INS_Address(ins);
     if (addr == 0x455BD3)
     {
	INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)printStatus,
		IARG_INST_PTR,
		IARG_REG_REFERENCE, REG_EAX,
		IARG_REG_REFERENCE, REG_EBX,
		IARG_REG_REFERENCE, REG_ECX,
		IARG_REG_REFERENCE, REG_EDX,
		IARG_REG_REFERENCE, REG_EDI,
		IARG_REG_REFERENCE, REG_ESI,
		IARG_REG_REFERENCE, REG_R8B,
		IARG_END);
     } 
}

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "Count " << icount << endl;
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
