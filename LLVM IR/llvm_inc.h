#include "globals.h"
#include "parse.h"
#include "scan.h"
#include "util.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <iostream>
#include <map>
#include<stack>
#include<fstream>
#include <memory>
#include <string>
#include<iomanip>

using namespace std;



using namespace llvm;
typedef string function_name;
typedef string variName;
typedef string arrName;

#define MYINT 1
#define MYARRAY 2
/*for synbol table */
struct VarInfo {
	string varName;
	int decLineNo;
	vector<int> useLineNo;
	bool isParam;
	int type;		//INT  or ARRAY
	int length;		//��������飬��¼��С
};
/*---------------------------------------------*/
struct forFunUsedInfo {
	string caller;
	int lineno;
};
struct qtlocalVar {
	map<variName, Value*> localVariableAlloca;
	map<variName, Value*> localVariableLoad;
	map<arrName, Value*> arrayAlloca;
};
struct function_Info {
	int funcDecLineNo;
	vector<qtlocalVar> localVarStack;
	/*--------------------------------------------*/
	/* ����ֻΪ��ӡ���ű� */
	/*map<variName, int>    funcParamDecLineNo;
	map<variName, vector<int>> funcParamUseLineNo;*/
	vector<string>  used_funName;   //����������б����õĺ���
	vector<forFunUsedInfo> beUsed;	// ����������ĸ��������������
	map<variName, VarInfo>  localVarInfo;
	/*------------------------------------------------------------*/
	Function* function;
}; 
