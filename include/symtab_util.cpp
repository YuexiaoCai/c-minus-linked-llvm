#include "llvm_inc.h"
/*
TODO
all print control use setwithe
*/
extern char program[100];
int printTab(map<function_name, function_Info> functions) {
	cout << "Code source name: " << program << endl;
	for (auto iter = functions.begin(); iter != functions.end(); iter++) {
		cout << "*******   Function Declaratioin Information     *******" << endl;
		cout << "Funciton name: " << iter->first << endl;
		cout << "Declarate at line: " << iter->second.funcDecLineNo << endl;
		/*   now print params infomation    */
		cout << "Function's parameters' infomation : " << endl;
		cout << "DecLine"<<"       "<<"Name" << "        " << "Type" << "           " << "ArrayLength" << "           " << "UsedLines" << endl;
		for (auto iter_param = iter->second.localVarInfo.begin(); iter_param != iter->second.localVarInfo.end(); iter_param++) {
			if (iter_param->second.isParam == true) {
				cout << iter_param->second.decLineNo << "       ";
				cout << iter_param->first << "        ";
				if (iter_param->second.type == INT) {
					cout << "int" << "           ";
				}
				else if (iter_param->second.type == ARRAY) {
					cout<<"array"<< "           " << iter_param->second.length << "           ";
				}
				else {
					//now can not occur
				}
				/*print used lines*/
				for (int i = 0; i < iter_param->second.useLineNo.size(); i++) {
					cout << iter_param->second.useLineNo[i] << ',';
				}
				cout << endl;
			}
		}
		for (auto iter_param = iter->second.localVarInfo.begin(); iter_param != iter->second.localVarInfo.end(); iter_param++) {
			if (iter_param->second.isParam != true) {
				cout << iter_param->second.decLineNo << "       ";
				cout << iter_param->first << "        ";
				if (iter_param->second.type == INT) {
					cout << "int" << "           ";
				}
				else if (iter_param->second.type == ARRAY) {
					cout << "array" << "           " << iter_param->second.length << "           ";
				}
				else {
					//now can not occur
				}
				/*print used lines*/
				for (int i = 0; i < iter_param->second.useLineNo.size(); i++) {
					cout << iter_param->second.useLineNo[i] << ',';
				}
				cout << endl;
			}
		}
		cout << endl;
		/*print functions are used in this function*/
		cout << "***   Functions are used in this function  ***" << endl;
		for (int i = 0; i < iter->second.used_funName.size(); i++) {
			cout << iter->second.used_funName[i] << "     ";
		}
		cout << "***   This function is used at WHERE  ***" << endl;
		cout << "Caller       LineNo" << endl;
		for (int i = 0; i < iter->second.beUsed.size(); i++) {
			cout << iter->second.beUsed[i].caller << "     " << iter->second.beUsed[i].lineno << endl;
		}
	}
}