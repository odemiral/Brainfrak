#include "Compiler.h"


Compiler::Compiler(string brainFInput) :
	m_context(llvm::getGlobalContext())
{
	m_bfInput = brainFInput;
	initMembers();
}

Compiler::Compiler(ifstream& bfFile) : m_context(llvm::getGlobalContext())
{
	void readFromFile(ifstream& bfFile);
	initMembers();
}

/* read the contents of bfFile into bfInput */
void Compiler::readFromFile(ifstream& bfFile)
{
	while (std::getline(bfFile, m_bfInput));
}

void Compiler::initMembers()
{
	m_module = std::make_unique<llvm::Module>("BrainFrak", m_context);
	m_builder = std::make_unique<IRBuilder<>>(m_context);

	/*Set input and data types. */
	m_charSize = IntegerType::getInt8Ty(m_context);     //size of char in the system, will be size of cellPtr
	m_intPtrSize = IntegerType::getInt32Ty(m_context);  // getInt64PtrTy or 32? 

	/* constants required by brainfF (0,1,-1 with the size of ptr) */
	m_one = ConstantInt::get(m_charSize, 1);
	m_minusOne = ConstantInt::get(m_charSize, -1);

	size_t memsize = 30000; /* TODO: Brainfuck has a fixed array size of 30,000 bytes, for now use this size, but in the future iterations implement malloc  */

	/* initialize brainf functions */
	initFunctions();

	/* Declare BasicBlock to write onto the main function, which will also be used to keep track of the current block. */
	m_block = BasicBlock::Create(m_context, "mainBlock", m_mainFunc);
	m_builder->SetInsertPoint(m_block);

	m_arrPointer = m_builder->CreateAlloca(m_charSize, ConstantInt::get(m_intPtrSize, memsize));
}

/* Initialize BrainFuck functions. */
void Compiler::initFunctions()
{
	/* Declare main brainf function (it's void type since it doesn't return anything.  */
	m_mainFunc = llvm::Function::Create(FunctionType::get(Type::getVoidTy(m_context), false),
		llvm::GlobalValue::InternalLinkage,
		"brainFMain",
		&(*m_module));

	/*Creating readByte & writeByte functions that are used by ',' and '.' commands respectively. */
	/*	declare void @writeByteFunc(i8)
	*	declare i8 @readByteFunc()
	*/
	m_readByteFunc = cast<Function>(m_module->getOrInsertFunction("getchar", FunctionType::get(m_charSize, false)));
	m_writeByteFunc = cast<Function>(m_module->getOrInsertFunction("putchar", FunctionType::get(Type::getVoidTy(m_context), m_charSize, false)));
}

/* Compile the BrainF code to IR. It will be called within JIT Compiler. */
void Compiler::compile()
{
	for (char &token : m_bfInput) {
		IRBuilder<> builder(m_block);
		//cout << "Current token: " << token << endl;
		switch (token) {
		case POINT_TO_RIGHT:
			incPtr(builder);
			break;
		case POINT_TO_LEFT:
			decPtr(builder);
			break;
		case INCREMENT_DATA:
			incData(builder);
			break;
		case DECREMENT_DATA:
			decData(builder);
			break;
		case OUTPUT_DATA:
			outputData(builder);
			break;
		case INPUT_DATA:
			inputData(builder);
			break;
		case LOOP_START:
			loopStart(builder);
			break;
		case LOOP_END:
			loopEnd(builder);
			break;
		case WHITESPACE:
			break;
		case NEW_LINE:
			break;
		case TAB:
			break;
		default:
			cout << "Error Undefined Token " << token << endl;
			exit(-1);
		}
	}

	/* return void instruction */
	IRBuilder<> returnBuilder(m_block);
	returnBuilder.CreateRetVoid();
}


/*Using ExecutionEngine JIT to compile into machine code on the fly
* TODO: Explore alternative JIT optimization techniques
*/
void Compiler::runJIT()
{
	InitializeNativeTarget();
	compile();
	auto engine = EngineBuilder(m_module.get()).create();
	if (!engine) {
		cout << "Fatal Error while creating the execution engine! " << endl;
		return;
	}

	/*JIT compilation */
	void* bfFuncPtr = engine->getPointerToFunction(m_mainFunc);
	reinterpret_cast<void(*)()>(bfFuncPtr)();
}

/* Shift pointer to right */
void Compiler::incPtr(llvm::IRBuilder<>& builder)
{
	m_arrPointer = builder.CreateGEP(m_arrPointer, m_one);
}

/* Shift pointer to left */
void Compiler::decPtr(llvm::IRBuilder<>& builder)
{
	m_arrPointer = builder.CreateGEP(m_arrPointer, m_minusOne);
}

/* Increase data by 1 byte at current pointer */
void Compiler::incData(llvm::IRBuilder<>& builder)
{
	Value *valAtPtr = builder.CreateLoad(m_arrPointer); //read from mem
	valAtPtr = builder.CreateAdd(valAtPtr, m_one); //add inst
	builder.CreateStore(valAtPtr, m_arrPointer); //save to mem
}

/* Decrease data by 1 byte at current pointer */
void Compiler::decData(llvm::IRBuilder<>& builder)
{
	//load from mem, add inst, save to mem -> load, add, create
	Value *valAtPtr = builder.CreateLoad(m_arrPointer); //read from mem
	valAtPtr = builder.CreateSub(valAtPtr, m_one); //sub inst
	builder.CreateStore(valAtPtr, m_arrPointer); //save to mem
}

/* get 1 byte input and store it at current pointer. Implemented it for the sake of completion, chances are, this op won't be useful to anyone. */
void Compiler::inputData(llvm::IRBuilder<>& builder)
{
	Value *input = builder.CreateCall(m_readByteFunc, "Read 1 Byte"); //read 1 byte.
	builder.CreateStore(input, m_arrPointer); //save input to mem valAtPtr points to
}

/* Write 1 byte to output */
void Compiler::outputData(llvm::IRBuilder<>& builder)
{
	Value *valAtPtr = builder.CreateLoad(m_arrPointer, "Write 1 Byte");
	builder.CreateCall(m_writeByteFunc, valAtPtr);
}


void Compiler::loopStart(llvm::IRBuilder<>& builder)
{
	// Construct loop info
	BFLoop bfLoop;
	bfLoop.blockBeforeLoop = m_block;
	bfLoop.loopHead = BasicBlock::Create(m_context, "Loop Head", m_mainFunc);
	bfLoop.loopBody = BasicBlock::Create(m_context, "Loop Body", m_mainFunc);
	bfLoop.valBeforeLoop = m_arrPointer;

	/* Create conditional branch instruction & insert the block (must not be null) */
	Value* cond = builder.CreateIsNotNull(builder.CreateLoad(m_arrPointer));
	builder.CreateCondBr(cond, bfLoop.loopHead, bfLoop.loopBody);

	/* Create a phi node and add it to stack we need phi nodes to keep track of nested loops and keep track of the state */
	IRBuilder<> loopBuilder(bfLoop.loopHead);
	bfLoop.phiNode = loopBuilder.CreatePHI(m_arrPointer->getType(), 0);
	bfLoop.phiNode->addIncoming(bfLoop.valBeforeLoop, bfLoop.blockBeforeLoop);

	//add the loop to stack
	m_bfLoops.push(bfLoop);
	//update block and set arpptr to point to the phi node.
	m_block = bfLoop.loopHead;
	m_arrPointer = bfLoop.phiNode;

}


void Compiler::loopEnd(llvm::IRBuilder<>& builder)
{
	/* If the loop stack is empty, there is no loop to end! */
	if (m_bfLoops.empty()) {
		cout << "Can't end a loop with no beginning " << endl;
		exit(-1);
	}

	BFLoop bfLoop = m_bfLoops.top();
	m_bfLoops.pop();

	Value* tempPtr = m_arrPointer;
	BasicBlock* tempBlock = m_block;

	/* Create conditional branch instruction & insert the block (must not be null) */
	Value* condition = builder.CreateIsNotNull(builder.CreateLoad(m_arrPointer));
	builder.CreateCondBr(condition, bfLoop.loopHead, bfLoop.loopBody);
	bfLoop.phiNode->addIncoming(tempPtr, tempBlock);
	m_block = bfLoop.loopBody;

	// Create a phi node
	IRBuilder<> loopBuilder(m_block);
	PHINode* phi = loopBuilder.CreatePHI(m_arrPointer->getType(), 0);
	phi->addIncoming(bfLoop.valBeforeLoop, bfLoop.blockBeforeLoop);
	phi->addIncoming(tempPtr, tempBlock);
	m_arrPointer = phi;
}
