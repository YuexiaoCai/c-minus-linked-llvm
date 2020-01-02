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
FILE* source;
FILE* listing;
int TraceScan = FALSE;
int Error1 = FALSE;
LLVMContext context;
Type* T32 = Type::getInt32Ty(context);
Type* myArray;
IRBuilder<> builder(context);
Module* module = new Module("theC", context);

string position = "global";  

map<function_name, function_Info> functions;
map<BasicBlock*, BasicBlock*> parentBlock;  
string temps;
Function* fun;  
BasicBlock* bb;

string funName;
Value* icmp;
TreeNode* localDec, * stmt_list;
Function* callFun;
//for control
BasicBlock* trueBB, * falseBB, * conti, * iterBB;
BasicBlock* retBB, *trueWhile, *falseWhile;
Value* additive;
Value* mul, * br;
vector<Value*> paras;
Value* call;
TreeNode* parasTree;
TreeNode* forGetCallPara;
//all for array and globals variable
int paramsLen;
vector<Type*> functionParams;
vector<string> functionParamsNames;
vector<Value*> callFunParamsLoad;
void getIR(TreeNode* tree);
stack<BasicBlock*> trueBBforSelect, falseBBforSelect, contiBBForSelect, iterBBforWhile, trueBBforWhile, falseBBforWhile;
function_Info newFunction() {
	function_Info t;		//分离出来，方便改进
	return t;
}
qtlocalVar newqtlocalVar() {
	qtlocalVar t;
	return t;
}
vector<Value*> args;

ArrayRef<Value*> offset;
Value* arrayDec;
Type* arrayParam = ArrayType::get(T32, 0);
Value* arrayTempEle;
int arrayDecLength;
forFunUsedInfo funUsedInfo;
char program[100];
/*        for globals var     */
Value* globalVarAddress;
/*-------------------------------------*/
/*        for qt dec          */
qtlocalVar tempcompount;
/*-------------------------------------*/
int main() {
	TreeNode* syntaxTree;
	strcpy(program, "code.c");
	source = fopen(program, "r");
	if (!source) {
		fprintf(stderr, "Can not open the c\n");
		exit(-1);
	}
	listing = stdout;
	syntaxTree = parse();

	fprintf(listing, "\nSyntax tree:\n");
	printTree(syntaxTree);
	getIR(syntaxTree);
	builder.ClearInsertionPoint();
	FILE* stream;
	stream = freopen("main.ll", "w", stdout);
	module->print(outs(), nullptr);
	delete module;
	return 0;
}

static const char* operatorString(TokenType op)
{
	if (op == INT) return "int";
	if (op == VOID) return "void";

	if (op == PLUS) return "+";
	if (op == MINUS) return "-";
	if (op == TIMES) return "*";
	if (op == OVER) return "/";

	if (op == LT) return "<";
	if (op == LE) return "<=";
	if (op == GT) return ">";
	if (op == GE) return ">=";
	if (op == EQ) return "==";
	if (op == NE) return "!=";

	return "";
}
NodeKind preNodeKind = ErrorK;
void getIR(TreeNode * tree) {

	
	for (; tree != NULL; tree = tree->sibling) {
		switch (tree->nodeKind) {
		case ErrorK:
			
			cout << "There is a ErrorK in the program!" << endl;
			preNodeKind = ErrorK;
			break;
		case VariableDeclarationK:
			cout << "Hi, I am VariableDeclarationK" << endl;
			preNodeKind = VariableDeclarationK;
			
			temps.clear();
			temps = tree->attr.varDecl._var->attr.ID;
			if (position=="global") 
			{
				if (globalVariableAlloca.count(temps) == 0)
				{
					GlobalVariable* glo = new GlobalVariable(T32, false, GlobalValue::LinkageTypes::ExternalLinkage);
					glo->setAlignment(10);
					glo->setInitializer(NULL);
				}
				else {}
			}
			else
			{

				if (functions[position].localVarStack[functions[position].localVarStack.size()-1].localVariableAlloca.count(temps) == 0)
				{
					// at here, stack never empty
					functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableAlloca[temps] = builder.CreateAlloca(T32);
					functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableLoad[temps] = NULL;
					/*for synbal table ---------------------------------------*/
					functions[position].localVarInfo[temps].varName = temps;
					functions[position].localVarInfo[temps].decLineNo = tree->lineno;
					functions[position].localVarInfo[temps].type = INT;
					functions[position].localVarInfo[temps].isParam = false;
					/*-----------------------------------------------------*/
				}
				else {}
			}
			//getIR(tree->sibling);
			break;

		case ArrayDeclarationK:
			cout << "Hi, I am ArrayDeclarationK" << endl;
			temps.clear();
			temps = tree->attr.arrDecl._var->attr.ID;
			if (position == "global") 
			{
				if (globalVariableAlloca.count(temps) == 0) 
				{
					//globalVariableAlloca[temps] = builder.CreateAlloca(Type::getArrayElementType(context));
				}
				else {}
			}
			else 
			{
				if (functions[position].localVarInfo.count(temps) == 0) {
					arrayDecLength = tree->attr.arrDecl._num->attr.NUM;
					myArray = ArrayType::get(T32, arrayDecLength);
					arrayDec = builder.CreateAlloca(myArray);
					functions[position].localVarStack[functions[position].localVarStack.size() - 1].arrayAlloca[temps] = arrayDec;
					/*----for synbal table   */
					functions[position].localVarInfo[temps].varName = temps;
					functions[position].localVarInfo[temps].decLineNo = tree->lineno;
					functions[position].localVarInfo[temps].type = ARRAY;
					functions[position].localVarInfo[temps].length = arrayDecLength;
					functions[position].localVarInfo[temps].isParam = false;
					/*-------------------------------------------------*/
				}
				else
				{
					//重复定义
				}
			}
			break;

		case FunctionDeclarationK:
			cout << "Hi, I am FunctionDeclarationK" << endl;
			funName = tree->attr.funcDecl._var->attr.ID;  
			functions[funName] = newFunction();
			functions[funName].funcDecLineNo = tree->lineno;
			functionParams.clear();
			functionParamsNames.clear();
			position = funName;
			tempcompount = newqtlocalVar();
			functions[position].localVarStack.push_back(tempcompount);
			if (tree->attr.funcDecl.type_spec->attr.TOK == INT) 
			{
			
				getIR(tree->attr.funcDecl.params);
				fun = Function::Create(FunctionType::get(T32, functionParams, false),
					GlobalValue::LinkageTypes::ExternalLinkage,
					funName, module);
				functions[position].function = fun;
			
				bb = BasicBlock::Create(context, "", fun);
				parentBlock[bb] = NULL; 
				
				builder.SetInsertPoint(bb);
				for (int i = 0; i < functionParams.size(); i++) 
				{
					if (functionParams[i] == T32) 
					{
						functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableAlloca[functionParamsNames[i]] = builder.CreateAlloca(T32);
						//functions[position].localVariableLoad[functionParamsNames[i]] = NULL;
						
					}
				}
				functions[position].function = fun;
				tree->attr.funcDecl.funEntry = bb;
				
				args.clear();
				for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++) 
				{
					args.push_back(arg);
				}
				for (int i = 0; i < args.size(); i++) 
				{
					if (functionParams[i] == T32) 
					{
						builder.CreateStore(args[i],
							functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableAlloca[functionParamsNames[i]]);
					}
				}
				
				getIR(tree->attr.funcDecl.cmpd_stmt);
			}
			else 
			{
				getIR(tree->attr.funcDecl.params);
				fun = Function::Create(FunctionType::get(Type::getVoidTy(context), functionParams, false),
					GlobalValue::LinkageTypes::ExternalLinkage,
					funName, module);
				functions[position].function = fun;
				bb = BasicBlock::Create(context, "", fun);
				
				builder.SetInsertPoint(bb);
				for (int i = 0; i < functionParams.size(); i++) 
				{
					if (functionParams[i] == T32) 
					{
						functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableAlloca[functionParamsNames[i]] = builder.CreateAlloca(T32);
						//functions[position].localVariableLoad[functionParamsNames[i]] = NULL;
					}
					else
					{

					}
				}
				functions[position].function = fun;
				
				tree->attr.funcDecl.funEntry = bb;
				
				args.clear();
				for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++) 
				{
					args.push_back(arg);
				}
				for (int i = 0; i < args.size(); i++) 
				{
					builder.CreateStore(args[i],
						functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableAlloca[functionParamsNames[i]]);
				}
				
				getIR(tree->attr.funcDecl.cmpd_stmt);
			}
			// out from function dec
			position = "global";
			functions[position].localVarStack.pop_back();
			break;

		case VariableParameterK:
			cout << "Hi, I am VariableParameterK" << endl;
			temps = tree->attr.varParam._var->attr.ID;
			if (functions[position].localVarInfo.count(temps) == 0) {
				functionParams.push_back(T32);
				functionParamsNames.push_back(temps);
				/*==============================================*/
				functions[position].localVarInfo[temps].decLineNo = tree->lineno;
				functions[position].localVarInfo[temps].isParam = true;
				functions[position].localVarInfo[temps].type = INT;
				functions[position].localVarInfo[temps].varName = temps;
				/*==============================================*/
			}
			else {
				//变量重复定义
			}
			break;

		case ArrayParameterK:
			cout << "Hi, I am ArrayParameterK" << endl;
			temps = tree->attr.varParam._var->attr.ID;
			if (functions[position].localVarInfo.count(temps) == 0) {
				functionParams.push_back(arrayParam);
				functionParamsNames.push_back(temps);
				functions[position].localVarInfo[temps].decLineNo = tree->lineno;
				functions[position].localVarInfo[temps].isParam = true;
				functions[position].localVarInfo[temps].type = ARRAY;
				functions[position].localVarInfo[temps].varName = temps;
			}
			else {
				//重复定义
			}
			break;

		case CompoundStatementK:
			cout << "Hi, I am CompoundStatementK" << endl;
			tempcompount = newqtlocalVar();
			functions[position].localVarStack.push_back(tempcompount);
			localDec = tree->attr.cmpdStmt.local_decl;
			getIR(localDec);
			stmt_list = tree->attr.cmpdStmt.stmt_list;
			getIR(stmt_list);
			functions[position].localVarStack.pop_back();
			break;

		case ExpressionStatementK:
			cout << "Hi, I am ExpressionStatementK" << endl;
		
			getIR(tree->attr.exprStmt.expr);
			tree->Load = tree->attr.exprStmt.expr->Load;
			break;

		case SelectionStatementK:
			cout << "Hi, I am SelectionStatementK" << endl;
			trueBB = BasicBlock::Create(context, "", fun);
			trueBBforSelect.push(trueBB);
			falseBB = BasicBlock::Create(context, "", fun);
			falseBBforSelect.push(falseBB);
			conti = BasicBlock::Create(context, "", fun);
			contiBBForSelect.push(conti);
			getIR(tree->attr.selectStmt.expr);
			if (tree->attr.selectStmt.expr->nodeKind == ComparisonExpressionK)
			{
				icmp = tree->attr.selectStmt.expr->Load;
			}
			else 
			{
				icmp = builder.CreateICmpNE(tree->attr.selectStmt.expr->Load, CONST(0));
			}
			trueBB = trueBBforSelect.top(); trueBBforSelect.pop();
			falseBB = falseBBforSelect.top(); 
			br = builder.CreateCondBr(icmp, trueBB, falseBB);
			builder.SetInsertPoint(trueBB);
			getIR(tree->attr.selectStmt.if_stmt);
			if (preNodeKind != ReturnStatementK) 
			{
				conti = contiBBForSelect.top();
				builder.CreateBr(conti);
			}
			falseBB = falseBBforSelect.top();	
			falseBBforSelect.pop();
			builder.SetInsertPoint(falseBB);
			
				getIR(tree->attr.selectStmt.else_stmt);
				if (preNodeKind != ReturnStatementK)
				{
					conti = contiBBForSelect.top();
					builder.CreateBr(conti);
				}
				conti = contiBBForSelect.top();  
				contiBBForSelect.pop();
				builder.SetInsertPoint(conti);
			
			break;

		case IterationStatementK:
			cout << "Hi, I am IterationStatementK" << endl;
			iterBB = BasicBlock::Create(context, "", fun);
			iterBBforWhile.push(iterBB);
			builder.CreateBr(iterBB);
			builder.SetInsertPoint(iterBB);
			getIR(tree->attr.iterStmt.expr);
			if (tree->attr.iterStmt.expr->nodeKind == ComparisonExpressionK) 
			{
				icmp = tree->attr.iterStmt.expr->Load;
			}
			else 
			{
				icmp = builder.CreateICmpNE(tree->attr.iterStmt.expr->Load, CONST(0));
			}
			trueWhile = BasicBlock::Create(context, "", fun);
			trueBBforWhile.push(trueWhile);
			falseWhile = BasicBlock::Create(context, "", fun);
															
			falseBBforWhile.push(falseWhile);
			trueWhile = trueBBforWhile.top(); 
			trueBBforWhile.pop();
			falseWhile = falseBBforWhile.top();			
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
				getIR(tree->attr.retStmt.expr);
				functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableLoad["return"] = tree->attr.retStmt.expr->Load;
				builder.CreateRet(functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableLoad["return"]);
				preNodeKind = ReturnStatementK;
			}
			else 
			{
				builder.CreateRetVoid();
				preNodeKind = ReturnStatementK;
			}
			break;

		case AssignExpressionK:
			cout << "Hi, I am AssignExpressionK" << endl;
			//TODO，应该检查变量在哪
			if (tree->attr.assignStmt._var->nodeKind == VariableK) 
			{
				getIR(tree->attr.assignStmt.expr);
				builder.CreateStore(builder.CreateLoad(tree->attr.assignStmt.expr->Alloca),
					functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableAlloca[tree->attr.assignStmt._var->attr.ID]);
			}
			else if (tree->attr.assignStmt._var->nodeKind == ArrayK)
			{
				getIR(tree->attr.assignStmt.expr);
				getIR(tree->attr.assignStmt._var->attr.arr.arr_expr);
				temps = tree->attr.assignStmt._var->attr.arr._var->attr.ID;
				arrayTempEle = builder.CreateGEP(functions[position].localVarStack[functions[position].localVarStack.size() - 1].arrayAlloca[temps],
								{ CONST(0), tree->attr.assignStmt._var->attr.arr.arr_expr->Load });
				builder.CreateStore(tree->attr.assignStmt.expr->Load, arrayTempEle);
			}
			break;

		case ComparisonExpressionK:
			cout << "Hi, I am ComparisonExpressionK" << endl;
			getIR(tree->attr.cmpExpr.lexpr);
			getIR(tree->attr.cmpExpr.rexpr);
			//tree->attr.cmpExpr.lexpr->Load = builder.CreateLoad(tree->attr.cmpExpr.lexpr->Alloca);
			//tree->attr.cmpExpr.rexpr->Load = builder.CreateLoad(tree->attr.cmpExpr.rexpr->Alloca);
			switch (tree->attr.cmpExpr.op->attr.TOK)
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
			tree->Load = icmp;
			break;

		case AdditiveExpressionK:
			cout << "Hi, I am AdditiveExpressionK" << endl;
			getIR(tree->attr.addExpr.lexpr);
			getIR(tree->attr.addExpr.rexpr);
			switch (tree->attr.addExpr.op->attr.TOK)
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


		case MultiplicativeExpressionK:
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
			cout << "Hi, I am VariableK" << endl;
			temps = tree->attr.ID;
//TODO，检查变量最先在哪
			for(auto iter = functions[position].localVarStack.end()-1; iter!=functions[position].localVarStack.begin()-1; iter--)
			if (functions[position].localVarInfo.count(temps) != 0) {
				functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableLoad[temps] =
					builder.CreateLoad(functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableAlloca[temps]);
				if (tree->Alloca == NULL)
				{
					tree->Alloca = builder.CreateAlloca(T32);
				}
				builder.CreateStore(functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableLoad[temps], tree->Alloca);
				tree->Load = functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableLoad[temps];
				/*============================================================*/
				functions[position].localVarInfo[temps].useLineNo.push_back(tree->lineno);
			}
			else {
				//变量未定义
			}
			break;

		case ArrayK:
			cout << "Hi, I am ArrayK" << endl;
			if (tree->Alloca == NULL)
			{
				tree->Alloca = builder.CreateAlloca(T32);
			}
			getIR(tree->attr.arr.arr_expr);
			temps = tree->attr.arr._var->attr.ID;
//TODO，检查变量最先在哪

			if (functions[position].localVarInfo.count(temps) != 0) {
				arrayTempEle = builder.CreateGEP(functions[position].localVarStack[functions[position].localVarStack.size() - 1].arrayAlloca[temps],
					{ CONST(0), tree->attr.arr.arr_expr->Load });
				builder.CreateStore(builder.CreateLoad(arrayTempEle), tree->Alloca);
				tree->Load = builder.CreateLoad(tree->Alloca);
				functions[position].localVarInfo[temps].useLineNo.push_back(tree->lineno);
			}
			break;

		case CallK:
			cout << "Hi, I am CallK" << endl;
			temps = tree->attr.call._var->attr.ID;
			if (functions.count(temps) == 0) 
			{
				cout << "ERROR! Function \"" << temps << "\" is no declaration！" << endl;
				exit(-1);
				//这个应该做成一个统一的出错处理程序
			}
			else {
				callFun = functions[temps].function;
				callFunParamsLoad.clear();
				getIR(tree->attr.call.expr_list);
				forGetCallPara = tree->attr.call.expr_list;
				while (forGetCallPara)
				{
					callFunParamsLoad.push_back(forGetCallPara->Load);
					forGetCallPara = forGetCallPara->sibling;
				}
				cout << callFunParamsLoad.size() << endl;
				call = builder.CreateCall(callFun, callFunParamsLoad);
				if (tree->Alloca == NULL)
				{
					tree->Alloca = builder.CreateAlloca(T32);
				}
				builder.CreateStore(call, tree->Alloca);
				tree->Load = builder.CreateLoad(tree->Alloca);
				/*================================================================*/
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
			tree->Load = builder.CreateLoad(tree->Alloca);
			break;

		case TokenTypeK:
			cout << "Hi, I am TokenTypeK" << endl;
		
			break;

		default:
			;
		}
	}
	
}
