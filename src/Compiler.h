/*  Written By Onur Demiralay Dec 2014 - Jan 2015 MIT License
 *  Github: @odemiral
 *  Brainfuck compiler with JIT support written in C++11 using LLVM
 */
#pragma once

#include <llvm\IR\IRBuilder.h>
#include <llvm\IR\BasicBlock.h>
#include <llvm\IR\Module.h>
#include <llvm\IR\LLVMContext.h>
#include <llvm\IR\Instructions.h>

#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/Support/raw_ostream.h>

#include <string>
#include <iostream>
#include <stack>
#include <memory>
#include <iostream>
#include <fstream>

using namespace llvm;
using namespace std;

/* Loop structure, all the data needed to keep track within BF loops. */
struct BFLoop
{
	Value* valBeforeLoop; // will point to m_arrPointer at the beginning of the loop
	
	PHINode* phiNode; //phi node instruction helps select value depending on the predecessor of the current loop block.
	
	/* loop blocks */
	BasicBlock* loopHead;
	BasicBlock* loopBody;
	BasicBlock* blockBeforeLoop;
};


/* Brainfuck tokens with default vals.
* if you want a full descriptions of what each token do, check out http://en.wikipedia.org/wiki/Brainfuck#Commands
*/
enum  BrainfuckTokens
{
	POINT_TO_RIGHT = '>',
	POINT_TO_LEFT = '<',
	INCREMENT_DATA = '+',
	DECREMENT_DATA = '-',
	OUTPUT_DATA = '.',
	INPUT_DATA = ',',
	LOOP_START = '[',
	LOOP_END = ']',
	WHITESPACE = ' ',
	NEW_LINE = '\n',
	TAB = '\t',
};


/* Big thanks to whoever asked and answered
* http://stackoverflow.com/questions/16692984/get-pointer-to-llvmvalue-previously-allocated-for-createload-function
* */


class Compiler
{
public:
	Compiler(Compiler const&) = delete;
	Compiler& operator=(Compiler const&) = delete;

	Compiler(string brainFInput);
	Compiler(ifstream& bfFile);
	void runJIT();
	void printIRInstructions();

private:
	void readFromFile(ifstream& bfFile);
	void initMembers(); 
	void compile();
	void initFunctions(); //initializes all bf functions
	string m_bfInput;

	/* llvm's builder,module and context objs */
	llvm::LLVMContext& m_context;
	unique_ptr<llvm::IRBuilder<>> m_builder; //llvm::IRBuilder<> m_builder;
	unique_ptr<llvm::Module> m_module; //    llvm::Module *m_module;


	Value *m_arrPointer; //pointer to brainfuck arr. each cell is a brainf pointer size of int pointer.

	//1 and -1 consts for BF,
	Value* m_one; 
	Value* m_minusOne;

	/* Declare BrainF Functions */
	llvm::Function* m_mainFunc;
	llvm::Function* m_readByteFunc;
	llvm::Function* m_writeByteFunc;

	BasicBlock *m_block; //BasicBlock for main brainF function, will be reuse to indicare the current block for each operations

	//switch to ConstIntType instead.
	Type *m_intPtrSize; //size of int ptr, which will be either 4 or 8 bytes depending on the architecture
	Type *m_charSize; //size of char, most likely will be 1 byte.


	/* http://llvm.org/docs/tutorial/LangImpl5.html#for-loop-expression */
	stack<BFLoop> m_bfLoops; //keeps track of all the BF loops (supports nested loops)


	/* BrainF IR Functions */
	void loopStart(llvm::IRBuilder<>& builder);		//[
	void loopEnd(llvm::IRBuilder<>& builder);		//]
	void incPtr(llvm::IRBuilder<>& builder);		//>
	void decPtr(llvm::IRBuilder<>& builder);		//<
	void incData(llvm::IRBuilder<>& builder);		//+
	void decData(llvm::IRBuilder<>& builder);		//-
	void inputData(llvm::IRBuilder<>& builder);		//,
	void outputData(llvm::IRBuilder<>& builder);	//.
};

