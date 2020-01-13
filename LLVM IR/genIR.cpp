/****************************************************/
/* File: IRGENERATE                                  */
/* using LLVM                                       */
/* Compiler Construction: Principles and Practice   */
/* Xiao Deng                                        */
/*                                                  */
/****************************************************/
#include "llvm_inc.h"
#define CONST(num)  ConstantInt::get(context, APInt(32, num)) 

int lineno = 0;
FILE* source;		//输入的程序源文件
FILE* listing;		//AST和符号表输出目的地
int TraceScan = FALSE;	//词法分析时要不要遇到一个token就马上产生一个输出
int Error1 = FALSE;		//记录目前有没有出错
LLVMContext context;	
Type* T32 = Type::getInt32Ty(context);		//32位 int 类型
Type* myArray;								//定义数组时使用的临时变量
IRBuilder<> builder(context);				//调用LLVM  API所需的主要对象，里面有很多关键函数
Module* module;								//一个源文件对应一个module

string position = "global";					//标记目前分析的位置属于全局，即不在函数内
map<variName, GlobalValue*> globalVariableAlloca;	//哈希法，记录全局变量的内存位置
map<function_name, function_Info> functions;		//记录一个函数的全部信息
string temps;					
Function* fun;			//指向一个函数，定义时使用
Function* callFun;		//函数调用时使用
BasicBlock* bb;			//一个basicblock是一个单入单出的基本块，并且对应一个标签，如 L1：

string funName;		
Value* icmp;			//存放布尔变量的结果，用于比较表达式
TreeNode* localDec, * stmt_list;	

//for control
BasicBlock* trueBB, * falseBB, * conti, * iterBB;
BasicBlock* retBB, *trueWhile, *falseWhile;
Value* additive;	//存加减法表达式的结果
Value* mul, *br;	//br存条件跳转的条件
Value* call;		//存放函数调用的返回值
TreeNode* forGetCallPara;	//为了遍历函数参数所需的临时变量
vector<Type*> functionParams;	//定义一个函数时，记录函数的变量类型
vector<string> functionParamsNames;	//与参数类型对应的参数名
vector<Value*> callFunParamsLoad;	//调用函数时，存放传递过去的参数
void getIR(TreeNode* tree);			//生成IR的函数，递归调用它生成IR
stack<BasicBlock*> trueBBforSelect, falseBBforSelect, contiBBForSelect, iterBBforWhile, trueBBforWhile, falseBBforWhile;
						//解决if、while重重嵌套所需的栈
function_Info newFunction() {
	function_Info t;		//分离出来，方便改进
	return t;
}
qtlocalVar newqtlocalVar() {	//记录一个复合语句内的变量信息
	qtlocalVar t;
	return t;
}
vector<Value*> args;		//为函数参数申请空间时所需临时变量

Value* arrayDec;
Type* arrayParam = ArrayType::get(T32, 0);	//数组类型函数参数临时变量
Value* arrayTempEle;			//指向一个数组元素的临时变量
int arrayDecLength;				//定义数组时记录数组长度的临时变量
forFunUsedInfo funUsedInfo;		
char program[100];				//源程序文件名
NodeKind preNodeKind = ErrorK; //
/*-------------------------------------*/
/*        处理嵌套          */
int nowptr; //	指向现在输出嵌套中的哪一层
/*-------------------------------------*/
int printTab(map<function_name, function_Info> functions);	//打印符号表函数
int main() 
{
	TreeNode* syntaxTree;		//保存AST的根
	strcpy(program, "code.c");
	source = fopen(program, "r");
	if (!source) 
	{
		fprintf(stderr, "Can not open the c\n");
		exit(-1);
	}
	module = new Module(program, context);	//对应于本程序源文件
	listing = stdout;		//指定打印所需信息到控制台
	syntaxTree = parse();	//生成AST
	
	fprintf(listing, "\nSyntax tree:\n");
	printTree(syntaxTree);
	getIR(syntaxTree);	
	printTab(functions);
	builder.ClearInsertionPoint();
	FILE* stream;
	stream = freopen("main.ll", "w", stdout);
	module->print(outs(), nullptr);			//将生成的中间代码IR输出到main.ll文件
	delete module;
	return 0;
}


void getIR(TreeNode * tree) 
{
	
	for (; tree != NULL; tree = tree->sibling) 
	{
		switch (tree->nodeKind) 
		{
		case ErrorK:
			cout << "There is a ErrorK in the program!" << endl;
			preNodeKind = ErrorK;
			//在输出AST时已处理过，不重复处理
			break;

		case VariableDeclarationK:
			cout << "Hi, I am VariableDeclarationK" << endl;
			temps.clear();
			temps = tree->attr.varDecl._var->attr.ID;	//记录变量名
			if (position=="global") 
			{
				if (globalVariableAlloca.count(temps) == 0)
				{
					/*GlobalVariable* glo = new GlobalVariable(T32, false, GlobalValue::LinkageTypes::ExternalLinkage);
					glo->setAlignment(10);
					glo->setInitializer(NULL);*/
					//没找到定义全局变量的API
					cout << "line： " << tree->lineno<<"    暂不支持使用全局变量！" << endl;
				}
				else {}//全局变量重复定义
			}
			else
			{
				nowptr = functions[position].localVarStack.size() - 1;   //指向本语句所在的嵌套层数
				if (functions[position].localVarStack[nowptr].localVariableAlloca.count(temps) == 0)
				{
					// 在这，栈绝不会是空
					functions[position].localVarStack[nowptr].localVariableAlloca[temps] = builder.CreateAlloca(T32);//为变量申请空间并记录
					functions[position].localVarStack[nowptr].localVariableLoad[temps] = NULL;			
					/*符号表记录信息--------*/
					functions[position].localVarInfo[temps].varName = temps;
					functions[position].localVarInfo[temps].decLineNo = tree->lineno;
					functions[position].localVarInfo[temps].type = MYINT;
					functions[position].localVarInfo[temps].isParam = false;
					/*-----------------------------------------------------*/
				}
				else 
				{
					cout << "line： " << tree->lineno << " 变量："<<temps<<"重复定义！" << endl;
				}
			}
			preNodeKind = VariableDeclarationK;
			break;

		case ArrayDeclarationK:
			cout << "Hi, I am ArrayDeclarationK" << endl;
			temps.clear();
			temps = tree->attr.arrDecl._var->attr.ID;		//数组名
			if (position == "global") 
			{
				if (globalVariableAlloca.count(temps) == 0) 
				{
					//globalVariableAlloca[temps] = builder.CreateAlloca(Type::getArrayElementType(context));
					cout << "line： " << tree->lineno << "    暂不支持使用全局变量！" << endl;
				}
				else {}
			}
			else 
			{
				nowptr = functions[position].localVarStack.size() - 1;
				if( functions[position].localVarStack[nowptr].arrayAlloca.count(temps) == 0 )
				{
					arrayDecLength = tree->attr.arrDecl._num->attr.NUM;		//数组大小
					myArray = ArrayType::get(T32, arrayDecLength);			
					arrayDec = builder.CreateAlloca(myArray);		//申请数组空间
					functions[position].localVarStack[nowptr].arrayAlloca[temps] = arrayDec;
					
					/*----for synbal table   */
					functions[position].localVarInfo[temps].varName = temps;
					functions[position].localVarInfo[temps].decLineNo = tree->lineno;
					functions[position].localVarInfo[temps].type = MYARRAY;
					functions[position].localVarInfo[temps].length = arrayDecLength;
					functions[position].localVarInfo[temps].isParam = false;
					/*-------------------------------------------------*/
				}
				else
				{
					cout << "line： " << tree->lineno << " 变量：" << temps << "重复定义！" << endl;
				}
			}
			break;

		case FunctionDeclarationK:
			cout << "Hi, I am FunctionDeclarationK" << endl;
			funName = tree->attr.funcDecl._var->attr.ID;  //函数名
			functions[funName] = newFunction();			 //用于记录函数全部信息的结构体
			functions[funName].funcDecLineNo = tree->lineno;
			functionParams.clear();
			functionParamsNames.clear();
			position = funName;
			functions[position].localVarStack.push_back(newqtlocalVar());	//函数体，也是一个新的复合语句
			getIR(tree->attr.funcDecl.params);			//获取函数的参数信息，函数类型记录在functionParams，参数名记录在functionParamsNames中
			if (tree->attr.funcDecl.type_spec->attr.TOK == INT) 
			{		//创建函数头
				fun = Function::Create(FunctionType::get(T32, functionParams, false),
					GlobalValue::LinkageTypes::ExternalLinkage,
					funName, module);
			}
			else if (tree->attr.funcDecl.type_spec->attr.TOK == VOID) 
			{
				fun = Function::Create(FunctionType::get(Type::getVoidTy(context), functionParams, false),
					GlobalValue::LinkageTypes::ExternalLinkage,
					funName, module);
			}
			functions[position].function = fun;
			bb = BasicBlock::Create(context, "", fun);		//函数体复合语句
			builder.SetInsertPoint(bb);						//表示接下来的代码属于这一函数体复合语句
			nowptr = functions[position].localVarStack.size() - 1;
			for (int i = 0; i < functionParams.size(); i++) //为函数参数申请空间
			{
				if (functionParams[i] == T32) 
				{
					functions[position].localVarStack[nowptr].localVariableAlloca[functionParamsNames[i]] = builder.CreateAlloca(T32);
				}
			}
			args.clear();
			for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++) 
			{
				args.push_back(arg);
			}
			for (int i = 0; i < args.size(); i++) //将函数调用者传过来的参数值存入参数的空间中
			{
				if (functionParams[i] == T32) 
				{
					builder.CreateStore(args[i],
						functions[position].localVarStack[nowptr].localVariableAlloca[functionParamsNames[i]]);
				}
			}
			getIR(tree->attr.funcDecl.cmpd_stmt);//产生函数体的代码
		
			// out from function dec
			functions[position].localVarStack.pop_back();	//一个复合语句结束
			position = "global";		//出了函数就属于全局区域
			break;

		case VariableParameterK:
			cout << "Hi, I am VariableParameterK" << endl;
			temps = tree->attr.varParam._var->attr.ID;	//参数名
			nowptr = functions[position].localVarStack.size() - 1;
			if(functions[position].localVarStack[nowptr].localVariableAlloca.count(temps)==0){
				functionParams.push_back(T32);			//参数类型
				functionParamsNames.push_back(temps);	//参数名
				/*=================== symbol table =======================*/
				functions[position].localVarInfo[temps].decLineNo = tree->lineno;
				functions[position].localVarInfo[temps].isParam = true;
				functions[position].localVarInfo[temps].type = MYINT;
				functions[position].localVarInfo[temps].varName = temps;
				/*==============================================*/
			}
			else {
				cout << "line： " << tree->lineno << " 变量：" << temps << "重复定义！" << endl;
			}
			break;

		case ArrayParameterK:
			cout << "Hi, I am ArrayParameterK" << endl;
			temps = tree->attr.varParam._var->attr.ID;
			nowptr = functions[position].localVarStack.size() - 1;
			if (functions[position].localVarStack[nowptr].arrayAlloca.count(temps) == 0) {
				functionParams.push_back(arrayParam);
				functionParamsNames.push_back(temps);
				/*---------symbol table -----------*/
				functions[position].localVarInfo[temps].decLineNo = tree->lineno;
				functions[position].localVarInfo[temps].isParam = true;
				functions[position].localVarInfo[temps].type = MYARRAY;
				functions[position].localVarInfo[temps].varName = temps;
				/*-------------------------------------------------------------------------------*/
			}
			else {
				cout << "line： " << tree->lineno << " 变量：" << temps << "重复定义！" << endl;
			}
			break;

		case CompoundStatementK:
			cout << "Hi, I am CompoundStatementK" << endl;
			functions[position].localVarStack.push_back(newqtlocalVar());//一个新的复合语句
			getIR(tree->attr.cmpdStmt.local_decl);
			getIR(tree->attr.cmpdStmt.stmt_list);
			functions[position].localVarStack.pop_back();	//本复合语句结束
			break;

		case ExpressionStatementK:
			cout << "Hi, I am ExpressionStatementK" << endl;
			getIR(tree->attr.exprStmt.expr);
			tree->Load = tree->attr.exprStmt.expr->Load;		//表达式的值（综合属性）记录到树结点上，供父节点使用
			break;

		case SelectionStatementK:
			cout << "Hi, I am SelectionStatementK" << endl;
			trueBB = BasicBlock::Create(context, "", fun);//标签，位于true基本块的首语句
			trueBBforSelect.push(trueBB);
			falseBB = BasicBlock::Create(context, "", fun);//标签，位于false基本块的首语句
			falseBBforSelect.push(falseBB);
			conti = BasicBlock::Create(context, "", fun);//标签，位于false基本快之后的第一条语句
			contiBBForSelect.push(conti);			//三个栈，为处理嵌套而生
			getIR(tree->attr.selectStmt.expr);		//产生true基本块的语句
			if (tree->attr.selectStmt.expr->nodeKind == ComparisonExpressionK)//条件是布尔语句则依据布尔值判断，否则非零即满足条件
			{
				icmp = tree->attr.selectStmt.expr->Load;
			}
			else 
			{
				icmp = builder.CreateICmpNE(tree->attr.selectStmt.expr->Load, CONST(0));//compare not equal
			}
			trueBB = trueBBforSelect.top(); trueBBforSelect.pop();
			falseBB = falseBBforSelect.top(); 
			br = builder.CreateCondBr(icmp, trueBB, falseBB);//满足条件则跳转到true基本快，否则false基本快，\
															 如果没有else语句，则falseBB和conti相当于同一个标签
			builder.SetInsertPoint(trueBB);			//产生true基本块的IR
			getIR(tree->attr.selectStmt.if_stmt);
			if (preNodeKind != ReturnStatementK) //如果true基本块中有return语句，则不用跳转到false基本块之后了，因为函数已经返回了
			{
				conti = contiBBForSelect.top();
				builder.CreateBr(conti);
			}
			falseBB = falseBBforSelect.top();	
			falseBBforSelect.pop();
			builder.SetInsertPoint(falseBB);
			
			getIR(tree->attr.selectStmt.else_stmt);
			if (preNodeKind != ReturnStatementK)	//同理
			{
				conti = contiBBForSelect.top();
				builder.CreateBr(conti);
			}
			conti = contiBBForSelect.top();  
			contiBBForSelect.pop();
			builder.SetInsertPoint(conti);//产生false基本块之后的语句的IR
			break;

		case IterationStatementK:
			cout << "Hi, I am IterationStatementK" << endl;
			iterBB = BasicBlock::Create(context, "", fun);//对应循环的条件判断基本块
			iterBBforWhile.push(iterBB);
			builder.CreateBr(iterBB);//到这里时，要无条件进入循环条件判断基本块，否则此基本块没有入口，程序无法继续执行
			builder.SetInsertPoint(iterBB);
			getIR(tree->attr.iterStmt.expr);
			if (tree->attr.iterStmt.expr->nodeKind == ComparisonExpressionK) //同理
			{
				icmp = tree->attr.iterStmt.expr->Load;
			}
			else 
			{
				icmp = builder.CreateICmpNE(tree->attr.iterStmt.expr->Load, CONST(0));
			}
			trueWhile = BasicBlock::Create(context, "", fun);//trueBBforWhile、falseBBforWhile两个栈为处理嵌套而生
			//trueBBforWhile.push(trueWhile);
			falseWhile = BasicBlock::Create(context, "", fun);
			falseBBforWhile.push(falseWhile);
			//trueWhile = trueBBforWhile.top(); 
			//trueBBforWhile.pop();
			//falseWhile = falseBBforWhile.top();			
			br = builder.CreateCondBr(icmp, trueWhile, falseWhile);
			builder.SetInsertPoint(trueWhile);
			getIR(tree->attr.iterStmt.loop_stmt);
			iterBB = iterBBforWhile.top();  
			iterBBforWhile.pop();
			builder.CreateBr(iterBB);
			falseWhile = falseBBforWhile.top();	
			falseBBforWhile.pop();
			builder.SetInsertPoint(falseWhile);
			break;

		case ReturnStatementK:
			cout << "Hi, I am ReturnStatementK" << endl;
			if (tree->attr.retStmt.expr != NULL)
			{
				getIR(tree->attr.retStmt.expr);//获取要返回的值
				//functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableLoad["return"] = tree->attr.retStmt.expr->Load;
				builder.CreateRet(tree->attr.retStmt.expr->Load);//综合属性
			}
			else 
			{
				builder.CreateRetVoid();
			}
			preNodeKind = ReturnStatementK;
			break;

		case AssignExpressionK:
			cout << "Hi, I am AssignExpressionK" << endl;
			nowptr = functions[position].localVarStack.size() - 1;
			if (tree->attr.assignStmt._var->nodeKind == VariableK) //左值是简单变量
			{
				getIR(tree->attr.assignStmt.expr);//获取右值
				bool flag = 0;
				temps = tree->attr.assignStmt._var->attr.ID;//左值变量名
				for (int i = nowptr; i >= 0; i--) //从嵌套的内层往外寻找变量的定义位置
				{
					if (functions[position].localVarStack[i].localVariableAlloca.count(temps) != 0) {//找到了
						builder.CreateStore(builder.CreateLoad(tree->attr.assignStmt.expr->Alloca),
											functions[position].localVarStack[i].localVariableAlloca[temps]);
						functions[position].localVarInfo[temps].useLineNo.push_back(tree->lineno);
						//存入左值对应的空间
						flag = 1;
						break;
					}
				}
				//处理变量未定义
				if (flag == 0) {
					cout << "line： " << tree->lineno << " 变量：" << temps << "未定义！" << endl;
				}
			}
			else if (tree->attr.assignStmt._var->nodeKind == ArrayK)	//左值是数组元素
			{
				int flag = 0;
				getIR(tree->attr.assignStmt.expr);
				getIR(tree->attr.assignStmt._var->attr.arr.arr_expr);//获取数组元素下标
				temps = tree->attr.assignStmt._var->attr.arr._var->attr.ID;//数组名
				for (int i = nowptr; i >= 0; i--) {			//从嵌套的内层往外寻找变量的定义位置
					if (functions[position].localVarStack[i].arrayAlloca.count(temps) != 0) {
						arrayTempEle = builder.CreateGEP(functions[position].localVarStack[i].arrayAlloca[temps],
							{ CONST(0), tree->attr.assignStmt._var->attr.arr.arr_expr->Load });	//找到该元素的位置
						builder.CreateStore(tree->attr.assignStmt.expr->Load, arrayTempEle);	//将右值存进去
						flag = 1;
						functions[position].localVarInfo[temps].useLineNo.push_back(tree->lineno);
						break;
					}
				}
				if (flag == 0) {
					cout << "line： " << tree->lineno << " 变量：" << temps << "未定义！" << endl;
				}
			}
			break;

		case ComparisonExpressionK:
			cout << "Hi, I am ComparisonExpressionK" << endl;
			getIR(tree->attr.cmpExpr.lexpr);//获取比较式的左值
			getIR(tree->attr.cmpExpr.rexpr);//右值
			//tree->attr.cmpExpr.lexpr->Load = builder.CreateLoad(tree->attr.cmpExpr.lexpr->Alloca);
			//tree->attr.cmpExpr.rexpr->Load = builder.CreateLoad(tree->attr.cmpExpr.rexpr->Alloca);
			switch (tree->attr.cmpExpr.op->attr.TOK)	//产生比较结果
			{
			case LT:icmp = builder.CreateICmpSLT(tree->attr.cmpExpr.lexpr->Load, 
				tree->attr.cmpExpr.rexpr->Load); 
				break;
			case LE:icmp = builder.CreateICmpSLE(tree->attr.cmpExpr.lexpr->Load,
				tree->attr.cmpExpr.rexpr->Load); 
				break;
			case GT:icmp = builder.CreateICmpSGT(tree->attr.cmpExpr.lexpr->Load, 
				tree->attr.cmpExpr.rexpr->Load); 
				break;
			case GE:icmp = builder.CreateICmpSGE(tree->attr.cmpExpr.lexpr->Load, 
				tree->attr.cmpExpr.rexpr->Load); 
				break;
			case EQ:icmp = builder.CreateICmpEQ(tree->attr.cmpExpr.lexpr->Load, 
				tree->attr.cmpExpr.rexpr->Load);
				break;
			case NE:icmp = builder.CreateICmpNE(tree->attr.cmpExpr.lexpr->Load,
				tree->attr.cmpExpr.rexpr->Load); 
				break;
			default:
				break;
			}
			tree->Load = icmp;	//存于树上，供父节点使用
			break;

		case AdditiveExpressionK:
			cout << "Hi, I am AdditiveExpressionK" << endl;
			getIR(tree->attr.addExpr.lexpr);	//表达式左运算数
			getIR(tree->attr.addExpr.rexpr);	//表达式右运算数
			switch (tree->attr.addExpr.op->attr.TOK)//产生运算结果
			{
			case PLUS:	additive = builder.CreateNSWAdd(tree->attr.addExpr.lexpr->Load,
				tree->attr.addExpr.rexpr->Load); 
				break;
			
			case MINUS:	additive = builder.CreateNSWSub(tree->attr.addExpr.lexpr->Load,
				tree->attr.addExpr.rexpr->Load); 
				break;
			default:
				break;
			}
			if (tree->Alloca == NULL) 
			{
				tree->Alloca = builder.CreateAlloca(T32);
			}
			builder.CreateStore(additive, tree->Alloca);
			tree->Load = builder.CreateLoad(tree->Alloca);
			break;


		case MultiplicativeExpressionK:		//同加减表达式
			cout << "Hi, I am MultiplicativeExpressionK" << endl;
			getIR(tree->attr.multExpr.lexpr);
			getIR(tree->attr.multExpr.rexpr);
			switch (tree->attr.addExpr.op->attr.TOK)
			{
			case TIMES:	additive = builder.CreateNSWMul(tree->attr.multExpr.lexpr->Load,
				tree->attr.multExpr.rexpr->Load); 
				break;
			case OVER:	additive = builder.CreateSDiv(tree->attr.multExpr.lexpr->Load, 
				tree->attr.multExpr.rexpr->Load); 
				break;
			default:
				break;
			}
			if (tree->Alloca == NULL)
			{
				tree->Alloca = builder.CreateAlloca(T32);
			}
			builder.CreateStore(additive, tree->Alloca);
			tree->Load = builder.CreateLoad(tree->Alloca);
			break;

		case VariableK:
		{
			cout << "Hi, I am VariableK" << endl;
			temps = tree->attr.ID;	//这里的变量一定是作为右值使用，即使用它的值而不是使用它的空间地址
			int flag = 0;
			nowptr = functions[position].localVarStack.size() - 1;
			for (int i = nowptr; i >= 0; i--) 
			{
				if (functions[position].localVarStack[i].localVariableAlloca.count(temps) != 0) 
				{
					if (tree->Alloca == NULL)
					{
						tree->Alloca = builder.CreateAlloca(T32);
					}//LLVM IR不支持将同一位置的值赋值给本位置，所以先取出来放到别的地方，再放回去，否则会报错
					builder.CreateStore(builder.CreateLoad(functions[position].localVarStack[i].localVariableAlloca[temps]), tree->Alloca);
					tree->Load = builder.CreateLoad(tree->Alloca);//把值取出来放到树上，供父节点使用
					/*=========================  for symbol table  =============================*/
					functions[position].localVarInfo[temps].useLineNo.push_back(tree->lineno);
					/*-------------------------------------------------------------------------------*/
					flag = 1;
					break;
				}
			}
			if (flag == 0) {
				cout << "line： " << tree->lineno << " 变量：" << temps << "未定义！" << endl;
			}
			break;
		}
		case ArrayK:		//操作同上
		{
			cout << "Hi, I am ArrayK" << endl;
			int flag = 0;
			if (tree->Alloca == NULL)
			{
				tree->Alloca = builder.CreateAlloca(T32);
			}
			getIR(tree->attr.arr.arr_expr);
			temps = tree->attr.arr._var->attr.ID;
			nowptr = functions[position].localVarStack.size() - 1;
			for (int i = nowptr; i >= 0; i--) 
			{
				if (functions[position].localVarStack[i].arrayAlloca.count(temps) != 0) 
				{
					arrayTempEle = builder.CreateGEP(functions[position].localVarStack[i].arrayAlloca[temps],
						{ CONST(0), tree->attr.arr.arr_expr->Load });
					builder.CreateStore(builder.CreateLoad(arrayTempEle), tree->Alloca);
					tree->Load = builder.CreateLoad(tree->Alloca);
					/*-----for symbol table -------------*/
					functions[position].localVarInfo[temps].useLineNo.push_back(tree->lineno);
					/*--------------------------------------------------------------------------*/
					flag = 1;
				}
			}
			if (flag == 0) {
				cout << "line： " << tree->lineno << " 变量：" << temps << "未定义！" << endl;
			}
			break;
		}

		case CallK:
			cout << "Hi, I am CallK" << endl;
			temps = tree->attr.call._var->attr.ID;//要调用的函数名
			if (functions.count(temps) == 0) 
			{
				cout << "ERROR! 函数 \"" << temps << "\" 未定义！" << endl;
			}
			else 
			{
				callFun = functions[temps].function;
				callFunParamsLoad.clear();
				getIR(tree->attr.call.expr_list);//求出参数们的值
				forGetCallPara = tree->attr.call.expr_list;
				while (forGetCallPara)//记录所有参数的值
				{
					callFunParamsLoad.push_back(forGetCallPara->Load);
					forGetCallPara = forGetCallPara->sibling;
				}
				//cout << callFunParamsLoad.size() << endl;
				call = builder.CreateCall(callFun, callFunParamsLoad);//函数调用
				if (tree->Alloca == NULL)
				{
					tree->Alloca = builder.CreateAlloca(T32);
				}
				builder.CreateStore(call, tree->Alloca);
				tree->Load = builder.CreateLoad(tree->Alloca);
				/*===============================symbol table============================*/
				functions[position].used_funName.push_back(temps);
				funUsedInfo.caller = position;
				funUsedInfo.lineno = tree->lineno;
				functions[temps].beUsed.push_back(funUsedInfo);
				/*=================================================================*/
			}
			break;

		case ConstantK:
			cout << "Hi, I am ConstantK" << endl;
			if (tree->Alloca == NULL)
			{
				tree->Alloca = builder.CreateAlloca(T32);
			}
			builder.CreateStore(CONST(tree->attr.NUM), tree->Alloca);
			tree->Load = builder.CreateLoad(tree->Alloca);//将常量值放到树上供父节点调用
			break;

		case TokenTypeK:
			cout << "Hi, I am TokenTypeK" << endl;
			break;

		default:
			;
		}
	}
	
}
