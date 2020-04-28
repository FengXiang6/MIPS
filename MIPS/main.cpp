//base requirement:
//	18 MIPS instructions:
//		add, sub, and, or, addi,
//		ori, sll, srl, lw, sw,
//		lui, slt, slti, beq, bne,
//		j, jal, jr
//	MIPS -> machine code
//	machine code -> MIPS
//strong requirement:
//	GUI
//	bgt, bge, blt, ble, move
//Stronger functions:
//	Supporting more instructions such as: 
//		Xor, Nor, Sra, Xori, Lb, Sb, Lh, SW, Jalr
//	Simulate execution of the MIPS assembly program in step-by-step 
//		way with a window to show the values of registers.
//	Simulate execution of the MIPS assembly program from the current MIPS
//		instruction to the end and with a window to show the values of registers
//		With reset to initialize the simulation.
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
	"jr", "jalr",
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
	"jr", "jalr"
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
	{"jalr",0x0000f809},{"jr",0b001000},
	//none
	{"syscall", 0x0000000c},
};
map<string, int> labelTable = {};
vector<int> machineCode = {};
vector<Instruction> mipsCode = {};

int tokenize(const string& line, string* tokenList);
insFormType getInsFormType(const string& ins);
int checkRegisterSyntax(const string& reg); //done
int checkImmediateSyntax(const string& inm, bool Sa = false);
void checkIRSyntax(const string& IR, int* reg = nullptr, int* ofs = nullptr);
void checkLabelSyntax(const string& label, bool operand = true);
int convert2Inm(const char* number);
string converInt2Hexstr(int num);
int Scan1(ifstream& code);
int Scan2(ifstream& code);
//inline void filter(const string* tokenList, int& curr, const string& op) {
//	curr++;
//	if (tokenList[curr] != op) curr--;
//}
inline bool isIns(const string& ins) { return insSet.count(ins); }
inline void readReg(const string& reg, int& reg_id) { reg_id = regFile[reg]; }

int main()
{
	ifstream code("hw4.asm");
	//ofstream out("token_res_1.txt");
	if (!code.is_open()) { 
		cout << "error: can not open the file!" << endl; 
		return 0; 
	}
	if (!Scan1(code)) return 0;
	Scan2(code);
	return 0;
}





int tokenize(const string& line, string* tokenList) {
	int id = 0;
	bool comment = false;
	bool readItem = false;
	for (int i = 0; i < MaxListLen; i++)tokenList[i].clear();
	for (int i = 0; i < line.length(); i++) {
		if (comment) {
			//tokenList[id].push_back(line[i]);
			break;
		}
		else if (line[i] == ' ' || line[i] == ',') {
			if (readItem) { id++; readItem = false; }
		}
		else if (line[i] == ':') { 
			if (readItem) { id++; readItem = false; }
			tokenList[id].push_back(line[i]);
			id++;
		}
		else if (line[i] == '#') { 
			if (readItem) {  id++; readItem = false; }
			//tokenList[id].push_back('#'), id++; 
			comment = true; 
		}
		else if (line[i] == '/') {
			if (readItem) { id++; readItem = false; }
			if (i + 1 < line.length() && line[i + 1] == '/') {
				i = i + 1;
				//tokenList[id].append("//"), id++;
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
void checkLabelSyntax(const string& label, bool operand) {
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
int checkRegisterSyntax(const string& reg) {
	if (regFile.count(reg))return regFile[reg];
	else throw Error("invalid register!");
}
int checkImmediateSyntax(const string& inm, bool Sa) {
	int ans;
	try {
		ans = convert2Inm(inm.c_str());
		if (Sa && (ans < 0 || ans >= 32)) {
			throw Error("operand out of range!");
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
	if (radix ^ 10 && len > 6) {
		throw Error("immediate out of range!");
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
	if (radix ^ 16 &&(ans < int(0xffff8000) || ans > int(0x00007fff))) {
		throw Error("immediate out of range!");
	}
	if (radix ^ 16 && ans < 0) {
		ans &= 0x0000ffff;
	}
	return ans;
}
int Scan1(ifstream& code) {
	int addr = 0x00000000;
	int lineNumber = 0;
	bool syntaxErr = false;
	string line;
	string tokenList[MaxListLen];
	insFormType insForm;
	int len;
	Instruction thisLine;

	while (!code.eof()) {
		bool readInstructDone = false;
		int curr = 0;
		bool thisLineErr = false;

		++lineNumber;
		getline(code, line);
		len = tokenize(line, tokenList);
		if (lineNumber == 39) {
			lineNumber = lineNumber;
		}
		while (!thisLineErr) {
			if (curr >= len)break;
			else if (readInstructDone)thisLineErr = true;
			else if (isIns(tokenList[curr])) {
				thisLine.clear();
				thisLine.op.append(tokenList[curr]);
				thisLine.insForm = insForm = getInsFormType(tokenList[curr++]);
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
						checkLabelSyntax(tokenList[curr]);
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
						checkLabelSyntax(tokenList[curr], true);
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
				if (curr + 1 < len&&tokenList[curr + 1] == ":") {
					try {
						checkLabelSyntax(tokenList[curr], false);
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
	if (syntaxErr) return 0;
	else return 1;
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
int Scan2(ifstream& code) {
	ofstream coe("a.coe");
	coe << "memory_initialization_radix=16;" << endl
		<< "memory_initialization_vector=" << endl;

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
				//binary = opCode[mipsCode[i].op];
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
				if (!mipsCode[i].label.empty()) {
					if (!labelTable.count(mipsCode[i].label)) {
						throw Error("undefined label name!");
					}
					else {
						mipsCode[i].immediate = (labelTable[mipsCode[i].label] - (addr + 4)) >> 2;
					}
				}
				if (mipsCode[i].immediate < int(0xffff8000) || mipsCode[i].immediate > int(0x00007fff)) {
					throw Error("at most brach in [-32768, 32768] from current instructions!");
				}
				mipsCode[i].immediate &= 0x0000ffff;
				binary |= mipsCode[i].immediate;
				break;
			case Tg:
				binary |= opCode[mipsCode[i].op] << 26;
				if (!mipsCode[i].label.empty()) {
					if (!labelTable.count(mipsCode[i].label)) {
						throw Error("undefined label name!");
					}
					else {
						mipsCode[i].immediate = labelTable[mipsCode[i].label] >> 2;
					}
				}
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
		coe.close();
		remove("a.coe");
		return 0;
	}
	for (int i = 0; i < mipsCode.size(); i++) {
		coe << converInt2Hexstr(machineCode[i]);
		if (i == mipsCode.size() - 1)coe << ";";
		else coe << ", ";
	}
	coe.close();
	return 1;
}