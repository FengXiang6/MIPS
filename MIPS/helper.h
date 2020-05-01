#pragma once
typedef enum {
	NONE,
	VERSION,
	HELP,
	ASSEMBLE,
	DISASSEMBLE,
	COE
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