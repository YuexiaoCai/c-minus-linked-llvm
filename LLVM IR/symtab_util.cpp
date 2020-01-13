#include "llvm_inc.h"

extern char program[100];
int printTab(map<function_name, function_Info> functions) {
	cout << "Code source name: " << program << endl;
	for (auto iter = functions.begin(); iter != functions.end(); iter++) {
		cout << "*******   Function Declaratioin Information     *******" << endl;
		cout << "Funciton name: " << iter->first << endl;
		//cout << "Declarate at line: " << iter->second.funcDecLineNo << endl;
		/*   now print params infomation    */
		cout << "Function's variables' infomation : " << endl;
		cout	<< setw(10) << setfill(' ') <<"DecLine"<<"       "\
				<< setw(10) << setfill(' ') << "Name" << "       " \
				<< setw(10) << setfill(' ') << "Type" << "       " \
				<< setw(10) << setfill(' ') << "ArrayLength" << "       " \
				<< setw(10) << setfill(' ') << "UsedLines" << endl;
		for (auto iter_param = iter->second.localVarInfo.begin(); iter_param != iter->second.localVarInfo.end(); iter_param++) {
			if (iter_param->second.isParam == true) {
				cout << setw(10) << setfill(' ') << iter_param->second.decLineNo << "       ";
				cout << setw(10) << setfill(' ') << iter_param->first << "       ";
				if (iter_param->second.type == MYINT) {
					cout << setw(10) << setfill(' ') << "int" << "       " << setw(10) << setfill(' ') << " "<< "       ";
				}
				else if (iter_param->second.type == MYARRAY) {
					cout << setw(10) << setfill(' ') <<"array"<< "       " << setw(10) << setfill(' ') << iter_param->second.length << "       ";
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
				cout << setw(10) << setfill(' ') << iter_param->second.decLineNo << "       ";
				cout << setw(10) << setfill(' ') << iter_param->first << "       ";
				if (iter_param->second.type == MYINT) {
					cout << setw(10) << setfill(' ') << "int" << "       "<<setw(10) << setfill(' ') << " " << "       ";
				}
				else if (iter_param->second.type == MYARRAY) {
					cout << setw(10) << setfill(' ') << "array" << "       " << setw(10) << setfill(' ') << iter_param->second.length << "       ";
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
		if (iter->second.used_funName.size() == 0) {
			cout << "     null       " << endl;
		}
		for (int i = 0; i < iter->second.used_funName.size(); i++) {
			cout << iter->second.used_funName[i] << "     ";
		}
		cout << "***   This function is used at WHERE  ***" << endl;
		cout << "Caller       LineNo" << endl;
		if (iter->second.beUsed.size() == 0) {
			cout << "     null       " << endl;
		}
		for (int i = 0; i < iter->second.beUsed.size(); i++) {
			cout << iter->second.beUsed[i].caller << "     " << iter->second.beUsed[i].lineno << endl;
		}
	}
}