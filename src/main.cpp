/* Written By Onur Demiralay Dec 2014 - Jan 2015 MIT License
*  Github: @odemiral
*  Brainfuck compiler with JIT support written in C++11 using LLVM
*/

#include <iostream>
#include <string>
#include <fstream>
using namespace std;


#include "Compiler.h"
int main(int argc, char *argv[]) 
{
	ifstream file;
	if (argc < 2 || argc > 3) {
		cout << "Error: Incorrect number of arguments" << endl;
		cout << "Usage: " << "BrainfuckJIT -f <path/to/file>" << " or " << "BrainfuckJIT <brainfuck code>" << endl;
		exit(-1);
	}
	else if (argc == 2) {
		string code(argv[1]);
		Compiler bfCompiler(code);
		bfCompiler.runJIT();


	}
	/*Currently it does nothing when parameter isn't -f, I might add more parameters to support different operations. */
	else if (argc == 3) {
		if (strcmp(argv[1], "-f") == 0) {
			file.open(argv[2]);
			Compiler bfCompiler(file);
			bfCompiler.runJIT();
		}
		else{
 		}
	}
	/* some sample brainfuck codes for you to try it out. */
	//string bfSoSayWeAll = "[-]>[-]<>++++++++ + [<++++++++ + >-]<++.>++++ + [<++++ + >-]<++ + .>++++++++ + [<---------> - ]<++.>++++++ + [<++++++ + >-]<++.>++++[<++++>-]<--.>++++ + [<++++ + >-]<-.>++++++++ + [<---------> - ]<--------.>++++++ + [<++++++ + >-]<++++++.>++++[<++++>-]<--.>++++++++[<-------->-]<---- - .>++++++[<++++++>-]<-- - .>++++++ + [<++++++ + >-]<------..>++++++++ + [<---------> - ]<++++++.";
	//string bfHelloWorld = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
	//string bfFibCode = "++++++++++++++++++++++++++++++++++++++++++++>++++++++++++++++++++++++++++++++>++++++++++++++++>>+<<[>>>>++++++++++<<[->+>-[>+>>]>[+[-<+>]>+>>]<<<<<<]>[<+>-]>[-]>>>++++++++++<[->-[>+>>]>[+[-<+>]>+>>]<<<<<]>[-]>>[++++++++++++++++++++++++++++++++++++++++++++++++.[-]]<[++++++++++++++++++++++++++++++++++++++++++++++++.[-]]<<<++++++++++++++++++++++++++++++++++++++++++++++++.[-]<<<<<<<.>.>>[>>+<<-]>[>+<<+>-]>[<+>-]<<<-]<<++...";

	return 0;
}