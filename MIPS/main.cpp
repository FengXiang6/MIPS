#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include <map>
#include <set>
using namespace std;

typedef enum {
	NONE,
	VERSION,
	HELP,
	ASSEMBLE,
	DISASSEMBLE,
	COE,
	HEX
} progMode;
typedef enum {
	ERROR,
	RdRsRt,
	RdRtSa,
	RtRsIm,
	RtIm,
	Rt_ImRs,
	Rs,
	None,
	RsRtTg,
	Tg
} insFormType;
typedef FILE File;
class Error {
public:
	Error(const string& err) :errmsg(err) {}
	void print() {
		cout << errmsg << endl;
	}
	void print(int lineNumber) {
		cout << "error at line " << lineNumber 
			<< " : " << errmsg << endl;
	}
private:
	string errmsg;
};
class Instruction {
public:
	Instruction() :label(""),op(""), rt(0),rd(0),rs(0),shamt(0),immediate(0),opcode(0) {}
	void clear() {
		rt = rd = rs = shamt = immediate = opcode;
		label.clear();
		op.clear();
	}
	int rt, rd, rs;
	int shamt, immediate, opcode;
	int lineNumber;
	string label, op;
	insFormType insForm;
};
#define MaxListLen 20

const set<string> insSet = {
	"add",	"addu",	"sub",	"subu",
	"and",	"or",	"xor",	"nor",
	"sllv",	"srlv",	"srav",	"slt",
	"sltu",
	"sll",	"srl",	"sra",
	"addi", "ori",	"addiu",
	"andi",	"xori", "slti",	"sltiu",
	"lui",
	"lw",	"lb",	"lbu",	"lh",
	"lhu",	"sw",	"sh",	"sb",
	"jr",
	"syscall",
	"beq", "bne",
	"j", "jal"
};
const set<string> insForm_RdRsRt = {
	"add",	"addu",	"sub",	"subu",
	"and",	"or",	"xor",	"nor",
	"sllv",	"srlv",	"srav",	"slt",
	"sltu"
};
const set<string> insForm_RdRtSa = {
	"sll",	"srl",	"sra"
};
const set<string> insForm_RtRsIm = {
	"addi", "ori",	"addiu",
	"andi",	"xori", "slti",	"sltiu"
};
const set<string> insForm_RtIm = {
	"lui"
};
const set<string> insForm_Rt_ImRs = {
	"lw",	"lb",	"lbu",	"lh",
	"lhu",	"sw",	"sh",	"sb"
};
const set<string> insForm_Rs = {
	"jr"
};
const set<string> insForm_None = {
	"syscall"
};
const set<string> insForm_RsRtTg = {
	"beq", "bne"
};
const set<string> insForm_Tg = {
	"j", "jal"
};
map<string, int> regFile = {
	{"$zero",0},{"$at",1},{"$v0",2},{"$v1",3},
	{"$a0",4},{"$a1",5},{"$a2",6},{"$a3",7},
	{"$t0",8},{"$t1",9},{"$t2",10},{"$t3",11},
	{"$t4",12},{"$t5",13},{"$t6",14},{"$t7",15},
	{"$s0",16},{"$s1",17},{"$s2",18},{"$s3",19},
	{"$s4",20},{"$s5",21},{"$s6",22},{"$s7",23},
	{"$t8",24},{"$t9",25},{"$k0",26},{"$k1",27},
	{"$gp",28},{"$sp",29},{"$fp",30},{"$ra",31},
	{"$0",0},{"$1",1},{"$2",2},{"$3",3},
	{"$4",4},{"$5",5},{"$6",6},{"$7",7},
	{"$8",8},{"$9",9},{"$10",10},{"$11",11},
	{"$12",12},{"$13",13},{"$14",14},{"$15",15},
	{"$16",16},{"$17",17},{"$18",18},{"$19",19},
	{"$20",20},{"$21",21},{"$22",22},{"$23",23},
	{"$24",24},{"$25",25},{"$26",26},{"$27",27},
	{"$28",28},{"$29",29},{"$30",30},{"$31",31}
};
map<string, int> opCode = {
	//RtRsIm
	{"addi",0b001000}, {"ori",0b001101},{"addiu",0b001001},
	{"andi",0b001100},{"xori",0b001110},{"slti",0b001011},
	{"sltiu",0b001011},
	//RtIm
	{"lui",0b001111},
	//Rt_ImRs
	{"lw",0b100011},{"lb",0b100000},{"lbu",0b100100},{"lh",0b100001},
	{"lhu",0b100101},{"sw",0b101011},{"sh",0b101001},{"sb",0b101000},
	//RsRtTg
	{"beq",0b000100}, {"bne",0b000101},
	//Tg
	{"j",0b000010},{"jal",0b000011}
};
map<string, int> funcCode = {
	//RdRsRt
	{"add",0b100000},{"addu",0b100001},{"sub",0b100010},{"subu",0b100011},
	{"and",0b100100},{"or",0b100101},{"xor",0b100110},{"nor",0b100111},
	{"sllv",0b000100},{"srlv",0b000110},{"srav",0b000111},
	{"slt",0b101010},{"sltu",0b101011},
	//RdRtSa
	{"sll",0b000000},{"srl",0b000010},{"sra",0b000011},
	//Rs
	{"jr",0b001000},
	//none
	{"syscall", 0b001100},
};
map<string, int> labelTable = {};
vector<int> machineCode = {};
vector<Instruction> mipsCode = {};
progMode mode = NONE;
string outFName = "a.out";
string filePath;

//check version
int Version();
//check help
int Help();
//convert .bin to .coe
int Tocoe();
//convet hexes to asm
int HexToAsm();
/*                       assemble .asm into .bin                         */
int Assemble();															 //
//=========================================================================
//1th scan for building label table and checking syntax					 //
int Scan1(ifstream& asmCode);											 //
//2th scan for writing asm into bin										 //
int Scan2(ifstream& asmCode);											 //
//tokenize a line(raw) in .asm into several key words					 //
int tokenize(const string& line, string* tokenList);					 //
int checkRegisterSyntax(const string& reg);								 //
//check immediate syntax, including its range, sa for 5bit shamt,		 //
//bit26 for 26bit address												 //
int checkImmediateSyntax(const string& inm, 							 //
	bool Sa = false, 													 //
	bool bit26 = false);												 //
//check Immediate(Register) syntax, it can read at the same time		 //
void checkIRSyntax(const string& IR, 									 //
	int* reg = nullptr, 												 //
	int* ofs = nullptr);												 //
//check syntax of label. permitIm if this position can be replaced with  //
//an immediate															 //
int checkLabelSyntax(const string& label,								 //
	bool bit26 = false,													 //
	bool permitIm = true,												 //
	bool operand = true);												 //
//=========================================================================
/*                   disassemble .bin into .asm                      */
int Disassemble();													 //
//=====================================================================
//getBitsOf(num, 1, 0) = num[1:0]									 //
int getBitsOf(const unsigned int& num, int a, int b);				 //
//disassemble a single binCode into stream asmf						 //
int Disassemble32bit(ofstream& asmf, unsigned int& binCode);		 //
//=====================================================================
/*                     helper functions                       */
insFormType getInsFormType(const string& ins);				  //
//convert char buffer to immediate, support decimal			  //
//and heximal which starts with 0x							  //
int convert2Inm(const char* number);						  //
//convert int to its hex code and return string				  //
string converInt2Hexstr(int num);							  //
//parse -h/-v... args from command line 					  //
void ParseArgs(int argc, char** argv);						  //
//return 32bit value of 8 hexes								  //
unsigned int hexes2ui(const string& s);							  //
//if the string is a symbol of instruction					  //
inline bool isIns(const string& ins) { 						  //
	return insSet.count(ins); 								  //
}															  //
//return the actual order number of the register			  //
inline void readReg(const string& reg, int& reg_id) { 		  //
	reg_id = regFile[reg]; 									  //
}															  //
//return correponding value of the single hex code			  //
inline unsigned int shex2ui(const char c) {					  //
	if (c >= '0'&&c <= '9')return c - '0';					  //
	else if (c >= 'a'&&c <= 'f')return c - 'a' + 10;		  //
	else if (c >= 'A'&&c <= 'F')return c - 'A' + 10;		  //
	else throw Error("illegal hex format!");				  //
}															  //
//==============================================================


int main(int argc, char** argv) {
	try { ParseArgs(argc, argv); }
	catch (Error& e) { e.print(); }
	if (mode == NONE) return 0;
	//check version
	else if (mode == VERSION)return Version();
	//check help
	else if (mode == HELP)return Help();
	//assemble files
	else if (mode == ASSEMBLE)return Assemble();
	//disassemble files
	else if (mode == DISASSEMBLE)return Disassemble();
	//convert bin to coe
	else if (mode == COE)return Tocoe();
	else if (mode == HEX) return HexToAsm();
	else return 0;
}



void ParseArgs(int argc, char** argv) {
	if (argc == 1) {
		throw Error("syntax error: please input correct arguments. -h or --help for help.");
	}
	else {
		for (size_t i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
				if (mode != NONE) throw Error("syntax: too many arguments.");
				mode = VERSION;
				return;
			}
			else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
				if (mode != NONE) throw Error("syntax: too many arguments.");
				mode = HELP;
				return;
			}
			else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--assemble") == 0) {
				if (mode != NONE) throw Error("syntax: too many arguments.");
				mode = ASSEMBLE;
				if (i == argc - 1) {
					throw Error("syntax error: missing source.");
				}
				else {
					filePath = argv[++i];
				}
			}
			else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--disassemble") == 0) {
				if (mode != NONE) throw Error("syntax: too many arguments.");
				mode = DISASSEMBLE;
				if (i == argc - 1) {
					throw Error("syntax error: missing source.");
				}
				else {
					filePath = argv[++i];
				}
			}
			else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--coe") == 0) {
				if (mode != NONE) throw Error("syntax: too many arguments.");
				mode = COE;
				if (i == argc - 1) {
					throw Error("syntax error: missing source.");
				}
				else {
					filePath = argv[++i];
				}
			}
			//specify the output file
			else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0) {
				if (i == argc - 1) {
					throw Error("syntax error: missing target.");
				}
				else {
					outFName = argv[++i];
				}
			}
			else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--hex") == 0) {
				if (mode != NONE) throw Error("syntax: too many arguments.");
				mode = HEX;
				if (i == argc - 1) {
					throw Error("syntax error: missing source.");
				}
				else {
					filePath = argv[++i];
				}
			}
			else {
				throw Error("syntax error: please input correct arguments. -h or --help for help.");
			}
		}
	}
}

unsigned int hexes2ui(const string & s)
{
	unsigned int ans = 0;
	int operand = 28;
	int i = 0;
	if (s.length() < 8) throw Error("instruction must be aligned in 8 hexes!");
	try {
		while (operand >= 0) {
			ans |= shex2ui(s[i++]) << operand;
			operand -= 4;
		}
	}
	catch (Error& e) {
		throw e;
	}
	return ans;
}

int Assemble() {
	//open .asm file
	ifstream asmCode(filePath);
	if (!asmCode.is_open()) {
		cout << "error: can not open the file!" << endl;
		return 0;
	}
	if (!Scan1(asmCode)) return 0;
	Scan2(asmCode);
	return 0;
}
int Scan1(ifstream& code) {
	//instruction address, or PC value
	int addr = 0x00000000;
	int lineNumber = 0;
	bool syntaxErr = false;
	//line in .asm file
	string line;
	//storing the result of tokenization
	string tokenList[MaxListLen];
	insFormType insForm;
	//len of token list
	int len;
	//key information of the instruction
	Instruction thisLine;

	while (!code.eof()) {
		bool readInstructDone = false;
		int curr = 0;
		bool thisLineErr = false;

		++lineNumber;
		getline(code, line);
		len = tokenize(line, tokenList);
		while (!thisLineErr) {
			//alread read all the tokens
			if (curr >= len)break;
			//extra token leads to error
			else if (readInstructDone)thisLineErr = true;
			else if (isIns(tokenList[curr])) {
				thisLine.clear();
				//thisLine.op = the symbol of MIPS instruction, like 'add'
				thisLine.op.append(tokenList[curr]);
				thisLine.insForm = insForm = getInsFormType(tokenList[curr++]);
				//corresponds which line in .asm file
				thisLine.lineNumber = lineNumber;
				if (insForm == Rs) {
					try {
						if (len - curr != 1) throw Error("wrong operand number!");
						thisLine.rs = checkRegisterSyntax(tokenList[curr++]);
						readInstructDone = true;
						addr += 4;
					}
					catch (Error& e) {
						e.print(lineNumber);
						thisLineErr = true;
					}
				}
				else if (insForm == Tg) {
					try {
						if (len - curr != 1) throw Error("wrong operand number!");
						thisLine.immediate = checkLabelSyntax(tokenList[curr], true, true, true);
						thisLine.label = tokenList[curr++];
						readInstructDone = true;
						addr += 4;
					}
					catch (Error& e) {
						e.print(lineNumber);
						thisLineErr = true;
					}
				}
				else if (insForm == RdRsRt) {
					try {
						if (len - curr != 3) throw Error("wrong operand number!");
						thisLine.rd = checkRegisterSyntax(tokenList[curr++]);
						thisLine.rs = checkRegisterSyntax(tokenList[curr++]);
						thisLine.rt = checkRegisterSyntax(tokenList[curr++]);
						readInstructDone = true;
						addr += 4;
					}
					catch (Error& e) {
						e.print(lineNumber);
						thisLineErr = true;
					}
				}
				else if (insForm == RtRsIm) {
					try {
						if (len - curr != 3) throw Error("wrong operand number!");
						thisLine.rt = checkRegisterSyntax(tokenList[curr++]);
						thisLine.rs = checkRegisterSyntax(tokenList[curr++]);
						thisLine.immediate = checkImmediateSyntax(tokenList[curr++]);
						readInstructDone = true;
						addr += 4;
					}
					catch (Error& e) {
						e.print(lineNumber);
						thisLineErr = true;
					}
				}
				else if (insForm == RdRtSa) {
					try {
						if (len - curr != 3) throw Error("wrong operand number!");
						thisLine.rd = checkRegisterSyntax(tokenList[curr++]);
						thisLine.rt = checkRegisterSyntax(tokenList[curr++]);
						thisLine.shamt = checkImmediateSyntax(tokenList[curr++], true);
						readInstructDone = true;
						addr += 4;
					}
					catch (Error& e) {
						e.print(lineNumber);
						thisLineErr = true;
					}
				}
				else if (insForm == RtIm) {
					try {
						if (len - curr != 2) throw Error("wrong operand number!");
						thisLine.rt = checkRegisterSyntax(tokenList[curr++]);
						thisLine.immediate = checkImmediateSyntax(tokenList[curr++]);
						readInstructDone = true;
						addr += 4;
					}
					catch (Error& e) {
						e.print(lineNumber);
						thisLineErr = true;
					}
				}
				else if (insForm == RsRtTg) {
					try {
						if (len - curr != 3) throw Error("wrong operand number!");
						thisLine.rs = checkRegisterSyntax(tokenList[curr++]);
						thisLine.rt = checkRegisterSyntax(tokenList[curr++]);
						thisLine.immediate = checkLabelSyntax(tokenList[curr]);
						thisLine.label.append(tokenList[curr++]);
						readInstructDone = true;
						addr += 4;
					}
					catch (Error& e) {
						e.print(lineNumber);
						thisLineErr = true;
					}
				}
				else if (insForm == Rt_ImRs) {
					try {
						thisLine.rt = checkRegisterSyntax(tokenList[curr++]);
						checkIRSyntax(tokenList[curr++], &(thisLine.rs), &(thisLine.immediate));
						readInstructDone = true;
						addr += 4;
					}
					catch (Error& e) {
						e.print(lineNumber);
						thisLineErr = true;
					}
				}
				else if (insForm == None) {
					readInstructDone = true;
					addr += 4;
				}
				if (!thisLineErr) {
					mipsCode.push_back(thisLine);
				}
			}
			else {
				//build table of label, every label is followed by a ':'
				if (curr + 1 < len&&tokenList[curr + 1] == ":") {
					try {
						//all false for definition checking
						checkLabelSyntax(tokenList[curr], false, false, false);
						labelTable[tokenList[curr]] = addr;
					}
					catch (Error& e) {
						e.print(lineNumber);
						thisLineErr = true;
					}
					curr = curr + 2;
				}
				else {
					Error e("unknow instruction!");
					e.print(lineNumber);
					thisLineErr = true;
				}
			}
		}
		syntaxErr = thisLineErr || false;
	}
	code.close();
	if (syntaxErr) {
		cout << "scan 1 failed" << endl;
		return 0;
	}
	else return 1;
}
int Scan2(ifstream& code) {
	ofstream bin(outFName, ios::binary);
	int lineNumber = 0;
	int addr = 0;
	bool compileErr = false;
	int len = mipsCode.size(), i = -1;
	while (++i < len) {
		int binary = 0;
		try {
			switch (mipsCode[i].insForm) {
			case ERROR:
				throw Error("unknow instruction!");
				break;
			case RdRsRt:
				binary |= mipsCode[i].rs << 21;
				binary |= mipsCode[i].rt << 16;
				binary |= mipsCode[i].rd << 11;
				binary |= funcCode[mipsCode[i].op];
				break;
			case RdRtSa:
				binary |= mipsCode[i].rt << 16;
				binary |= mipsCode[i].rd << 11;
				binary |= mipsCode[i].shamt << 6;
				binary |= funcCode[mipsCode[i].op];
				break;
			case RtRsIm:
				binary |= opCode[mipsCode[i].op] << 26;
				binary |= mipsCode[i].rs << 21;
				binary |= mipsCode[i].rt << 16;
				binary |= mipsCode[i].immediate;
				break;
			case RtIm:
				binary |= opCode[mipsCode[i].op] << 26;
				binary |= mipsCode[i].rt << 16;
				binary |= mipsCode[i].immediate;
				break;
			case Rt_ImRs:
				binary |= opCode[mipsCode[i].op] << 26;
				binary |= mipsCode[i].rs << 21;
				binary |= mipsCode[i].rt << 16;
				binary |= mipsCode[i].immediate;
				break;
			case Rs:
				binary |= mipsCode[i].rs << 21;
				binary |= funcCode[mipsCode[i].op];
				break;
			case None:
				binary |= funcCode[mipsCode[i].op];
				break;
			case RsRtTg:
				binary |= opCode[mipsCode[i].op] << 26;
				binary |= mipsCode[i].rs << 21;
				binary |= mipsCode[i].rt << 16;
				//this mean it is a label instead of a number
				if (!(mipsCode[i].immediate ^ 0xffffffff)) {
					if (!labelTable.count(mipsCode[i].label)) {
						throw Error("undefined label name!");
					}
					else {
						mipsCode[i].immediate = labelTable[mipsCode[i].label];
					}
				}
				mipsCode[i].immediate = (mipsCode[i].immediate - (addr + 4)) >> 2;
				//re-checking for illegle label jump, 16bit
				if (mipsCode[i].immediate < int(0xffff8000) || mipsCode[i].immediate > int(0x00007fff)) {
					throw Error("at most brach in [-32768, 32768] from current instructions!");
				}
				mipsCode[i].immediate &= 0x0000ffff;
				binary |= mipsCode[i].immediate;
				break;
			case Tg:
				binary |= opCode[mipsCode[i].op] << 26;
				if (!(mipsCode[i].immediate ^ 0xffffffff)) {
					if (!labelTable.count(mipsCode[i].label)) {
						throw Error("undefined label name!");
					}
					else {
						mipsCode[i].immediate = labelTable[mipsCode[i].label];
					}
				}
				mipsCode[i].immediate >>= 2;
				//re-checking for illegle label jump, 26bit
				if (mipsCode[i].immediate < int(0xfe000000) || mipsCode[i].immediate > int(0x00dfffff)) {
					throw Error("illegal address which is out of 26 bits range!");
				}
				mipsCode[i].immediate &= 0x03ffffff;
				binary |= mipsCode[i].immediate;
				break;
			}
			machineCode.push_back(binary);
		}
		catch (Error& e) {
			e.print(mipsCode[i].lineNumber);
			compileErr = true;
		}
		addr += 4;
	}
	if (compileErr) {
		bin.close();
		remove(outFName.c_str());
		return 0;
	}
	for (int i = 0; i < mipsCode.size(); i++) {
		bin.write((char*)&machineCode[i], sizeof(int));
	}
	bin.close();
	return 1;
}
int Version() {
	//cout << "MIPS assembler -- version 0.0.1" << endl;
	//add function of converting hexes to asm
	cout << "MIPS assembler -- version 0.0.2" << endl;
	return 0;
}
int Help() {
	cout << "    -h,--help          for help" << endl;
	cout << "    -v,--version       check information of version" << endl;
	cout << "    -a,--assemble      assemble files" << endl;
	cout << "    -d,--disassemble   disassemble files" << endl;
	cout << "    -o,--out           specify the name of outfile" << endl;
	cout << "    -c,--coe           convert binary file to coe" << endl;
	cout << "    -x,--hex           convert hexes segment to asm code" << endl;
	return 0;
}
int Tocoe() {
	int ins;
	ifstream code(filePath, ios::binary);
	if (!code.is_open()) {
		cout << "error: can not open the file!" << endl;
		return 0;
	}
	ofstream coe(outFName, ios::binary);
	coe << "memory_initialization_radix=16;" << endl
		<< "memory_initialization_vector=" << endl;
	code.read((char*)&ins, sizeof(int));
	if (code) coe << converInt2Hexstr(ins);
	else {
		code.close();
		coe.close();
		remove(outFName.c_str());
		return 0;
	}
	do {
		code.read((char*)&ins, sizeof(int));
		if (!code) {
			coe << ";";
			break;
		}
		else coe << "," << endl;
		coe << converInt2Hexstr(ins);
	} while (true);
	code.close();
	coe.close();
	return 0;
}
int HexToAsm()
{
	ifstream hex(filePath);
	if (!hex.is_open()) {
		cout << "error: can not open the file!" << endl;
		return 0;
	}
	ofstream asmf(outFName, ios::out);
	string line;
	while (getline(hex, line)) {
		try {
			unsigned int binCode = hexes2ui(line);
			Disassemble32bit(asmf, binCode);
		}
		catch (Error& e) {
			e.print();
			asmf.close();
			remove(outFName.c_str());
			return 1;
		}
	}
	asmf.close();
	return 0;
}
int Disassemble()
{
	//read in binary format
	ifstream bin(filePath, ios::binary | ios::in);
	if (!bin.is_open()) {
		cout << "error: can not open the file!" << endl;
		return 0;
	}
	//do not care sign
	unsigned int binCode = 0;
	ofstream asmf(outFName, ios::out);
	while (bin.read((char*)&binCode, sizeof(unsigned int))) {
		try {
			Disassemble32bit(asmf, binCode);
		}
		catch (Error& e) {
			e.print();
			asmf.close();
			remove(outFName.c_str());
			return 1;
		}
	}
	asmf.close();
	return 0;
}
int Disassemble32bit(ofstream & asmf, unsigned int & binCode)
{
	static int addr = 0;
	int opcd, funccd, rs, rt, rd, sa, im;
	rs = getBitsOf(binCode, 25, 21);
	if (!getBitsOf(binCode, 31, 26)) {
		funccd = getBitsOf(binCode, 5, 0);
		if (funccd == 0b001000) {
			asmf << "jr $" << rs;
		}
		else if (funccd == 0b001100) {
			asmf << "syscall";
		}
		else if (funccd == 0b000010) {
			rt = getBitsOf(binCode, 20, 16);
			rd = getBitsOf(binCode, 15, 11);
			sa = getBitsOf(binCode, 10, 6);
			asmf << "srl $" << rd << ", $" << rt << ", " << sa;
		}
		else if (funccd == 0b000000) {
			rt = getBitsOf(binCode, 20, 16);
			rd = getBitsOf(binCode, 15, 11);
			sa = getBitsOf(binCode, 10, 6);
			asmf << "sll $" << rd << ", $" << rt << ", " << sa;
		}
		else if (funccd == 0b000011) {
			rt = getBitsOf(binCode, 20, 16);
			rd = getBitsOf(binCode, 15, 11);
			sa = getBitsOf(binCode, 10, 6);
			asmf << "sra $" << rd << ", $" << rt << ", " << sa;
		}
		else {
			rt = getBitsOf(binCode, 20, 16);
			rd = getBitsOf(binCode, 15, 11);
			map<string, int>::iterator funccd_map_iter;
			for (funccd_map_iter = funcCode.begin(); funccd_map_iter != funcCode.end(); funccd_map_iter++) {
				if (funccd_map_iter->second == funccd) {
					asmf << funccd_map_iter->first << " ";
					asmf << "$" << rd << ", $" << rs << ", $" << rt;
					break;
				}
			}
			if (funccd_map_iter == funcCode.end()) {
				string msg = "unknow instruction: " + converInt2Hexstr(binCode);
				cout << "here1" << endl;
				throw Error(msg);
			}
		}
	}
	else {
		opcd = getBitsOf(binCode, 31, 26);
		map<string, int>::iterator opcd_map_iter;
		for (opcd_map_iter = opCode.begin(); opcd_map_iter != opCode.end(); opcd_map_iter++) {
			if (opcd_map_iter->second == opcd) break;
		}
		if (opcd_map_iter == opCode.end()) {
			string msg = "unknow instruction: " + converInt2Hexstr(binCode);
			throw Error(msg);
		}
		string insSym = opcd_map_iter->first;
		insFormType insType = getInsFormType(insSym);
		switch (insType) {
		case RtRsIm:
			rt = getBitsOf(binCode, 20, 16);
			im = getBitsOf(binCode, 15, 0);
			if (im & 0x00008000) im |= 0xffff0000;
			asmf << insSym << " $" << rt << ", $" << rs << ", " << im;
			break;
		case RtIm:
			rt = getBitsOf(binCode, 20, 16);
			im = getBitsOf(binCode, 15, 0);
			if (im & 0x00008000) im |= 0xffff0000;
			asmf << insSym << " $" << rt << ", " << im;
			break;
		case Rt_ImRs:
			rt = getBitsOf(binCode, 20, 16);
			im = getBitsOf(binCode, 15, 0);
			if (im & 0x00008000) im |= 0xffff0000;
			asmf << insSym << " $" << rt << ", " << im << "($" << rs << ")";
			break;
		case Rs:
			asmf << insSym << " $" << rs;
			break;
		case RsRtTg:
			//(labelTable[mipsCode[i].label] - (addr + 4)) >> 2;
			rt = getBitsOf(binCode, 20, 16);
			im = getBitsOf(binCode, 15, 0);
			if (im & 0x00008000) im |= 0xffff0000;
			asmf << insSym << " $" << rs << ", $" << rt << ", 0x" << converInt2Hexstr((im << 2) + addr + 4);
			break;
		case Tg:
			im = getBitsOf(binCode, 25, 0);
			asmf << insSym << " 0x" << converInt2Hexstr(im << 2);
			break;
		}
	}
	asmf << "\t\t#address=0x" << converInt2Hexstr(addr) << endl;
	addr += 4;
}
int getBitsOf(const unsigned int& num, int a, int b) {
	if (a < b || a > 31 || b < 0) return 0;
	int len = a - b, operand = 1;
	while (len--) {
		operand <<= 1;
		operand |= 1;
	}
	operand <<= b;
	return (num & operand) >> b;
}
int tokenize(const string& line, string* tokenList) {
	//filter [,] [space] and [comment]
	int id = 0;
	bool comment = false;
	bool readItem = false;
	for (int i = 0; i < MaxListLen; i++)tokenList[i].clear();
	for (int i = 0; i < line.length(); i++) {
		if (comment) {
			break;
		}
		else if (line[i] == ' ' || line[i] == ',' || line[i] == '\r' || line[i] == '\n' || line[i] == '\t') {
			if (readItem) { id++; readItem = false; }
		}
		else if (line[i] == ':') { 
			if (readItem) { id++; readItem = false; }
			tokenList[id].push_back(line[i]);
			id++;
		}
		else if (line[i] == '#') { 
			if (readItem) {  id++; readItem = false; }
			comment = true; 
		}
		else if (line[i] == '/') {
			if (readItem) { id++; readItem = false; }
			if (i + 1 < line.length() && line[i + 1] == '/') {
				i = i + 1;
				comment = true;
			}
			else {
				tokenList[id].append(","), id++;
			}
		}
		else {
			readItem = true;
			tokenList[id].push_back(line[i]);
		}
	}
	return id + (readItem ? 1 : 0);
}
insFormType getInsFormType(const string& ins) {
	if (insForm_RdRsRt.count(ins))return RdRsRt;
	if (insForm_RdRtSa.count(ins))return RdRtSa;
	if (insForm_RtRsIm.count(ins))return RtRsIm;
	if (insForm_RtIm.count(ins))return RtIm;
	if (insForm_Rt_ImRs.count(ins))return Rt_ImRs;
	if (insForm_Rs.count(ins))return Rs;
	if (insForm_None.count(ins)) return None;
	if (insForm_RsRtTg.count(ins)) return RsRtTg;
	if (insForm_Tg.count(ins)) return Tg;
	return ERROR;
}
int checkLabelSyntax(const string& label, bool bit26, bool permitIm, bool operand) {
	int ans = 0xffffffff;
	try {
		if (!operand && labelTable.count(label)) {
			throw Error("can not define a label twice!");
		}
		if (isdigit(label[0])) {
			throw Error("invalid syntax of label name!");
		}
		for (int i = 0; i < label.length(); i++) {
			if (!isalpha(label[i]) && label[i] != '_' && !isdigit(label[i])) {
				throw Error("invalid syntax of label name!");
			}
		}
	}
	catch (Error& e) {
		if (permitIm) {
			try {
				ans = checkImmediateSyntax(label, false, bit26);
			}
			catch (Error& ee) {
				throw ee;
			}
		}
		else throw e;
	}
	return ans;
}
int checkRegisterSyntax(const string& reg) {
	if (regFile.count(reg))return regFile[reg];
	else throw Error("invalid register!");
}
int checkImmediateSyntax(const string& inm, bool Sa, bool bit26) {
	int ans;
	try {
		ans = convert2Inm(inm.c_str());
		if (Sa) {
			if (ans < 0 || ans >= 32) {
				throw Error("operand out of range!");
			}
		}
		else {
			if (!bit26) {
				if (ans < int(0xffff8000) || ans > int(0x00007fff)) {
					throw Error("immediate out of 16 bit range!");
				}
				if (ans < 0) {
					ans &= 0x0000ffff;
				}
			}
			else {
				if (ans < int(0xfe000000) || ans > int(0x01ffffff)) {
					throw Error("immediate out of 26 bit range!");
				}
				if (ans < 0) {
					ans &= 0x03ffffff;
				}
			}
		}
	}
	catch (Error& e) {
		throw e;
	}
	return ans;
}
void checkIRSyntax(const string& IR, int* reg, int* ofs) {
	if (IR[IR.length()-1]!=')')throw Error("invalid operand!");
	int offset, regn;
	char number[20]; int np = 0;
	char regstr[20]; int rp = 0;
	bool readReg = false;
	int sp = 0;
	while (sp < IR.length()-1) {
		if (!readReg&&IR[sp] == '(')readReg = true;
		else if (!readReg) {
			number[np++] = IR[sp];
		}
		else {
			regstr[rp++] = IR[sp];
		}
		sp++;
	}
	number[np] = regstr[rp] = 0;
	try {
		regn = checkRegisterSyntax(regstr);
		offset = convert2Inm(number);
	}
	catch (Error& e) {
		throw e;
	}
	if (ofs) *ofs = offset;
	if (reg) *reg = regn;
}
int convert2Inm(const char* number) {
	int len, radix = 10, np = 0;
	int ans = 0;
	int sign = 1;
	bool start = true;
	for (len = 0; number[len]; len++);
	if (len >= 2&&number[0] == '0'&&number[1] == 'x') {
		radix = 16; np = 2;
	}
	while (np < len) {
		if (start&&radix == 10 && number[np] == '-')sign = -1;
		else {
			if (isdigit(number[np])) {
				ans = ans * radix + number[np] - '0';
			}
			else if (radix ^ 10 && 'A' <= number[np] && number[np] <= 'F') {
				ans = ans * radix + number[np] - 'A' + 10;
			}
			else if (radix ^ 10 && 'a' <= number[np] && number[np] <= 'f') {
				ans = ans * radix + number[np] - 'a' + 10;
			}
			else throw Error("invalid operand of immediate!");
		}
		start = false;
		np++;
	}
	ans = ans * sign;
	if (radix ^ 10 && (ans & 0xffff0000)) {
		throw Error("out of 16bit immediate range!");
	}
	if (radix ^ 10 && (ans&0x00008000)) {
		ans |= 0xffff0000;
	}
	return ans;
}
string converInt2Hexstr(int num) {
	string ans = "";
	int i = 0, tmp = num, hex;
	do {
		hex = (tmp & 0xf0000000) >> 28;
		if (hex >= 10) {
			ans.push_back('A' - 10 + hex);
		}
		else {
			ans.push_back(hex + '0');
		}
		tmp <<= 4;
	} while (++i < 8);
	return ans;
}