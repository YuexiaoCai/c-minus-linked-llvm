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
FILE* source;		//����ĳ���Դ�ļ�
FILE* listing;		//AST�ͷ��ű����Ŀ�ĵ�
int TraceScan = FALSE;	//�ʷ�����ʱҪ��Ҫ����һ��token�����ϲ���һ�����
int Error1 = FALSE;		//��¼Ŀǰ��û�г���
LLVMContext context;	
Type* T32 = Type::getInt32Ty(context);		//32λ int ����
Type* myArray;								//��������ʱʹ�õ���ʱ����
IRBuilder<> builder(context);				//����LLVM  API�������Ҫ���������кܶ�ؼ�����
Module* module;								//һ��Դ�ļ���Ӧһ��module

string position = "global";					//���Ŀǰ������λ������ȫ�֣������ں�����
map<variName, GlobalValue*> globalVariableAlloca;	//��ϣ������¼ȫ�ֱ������ڴ�λ��
map<function_name, function_Info> functions;		//��¼һ��������ȫ����Ϣ
string temps;					
Function* fun;			//ָ��һ������������ʱʹ��
Function* callFun;		//��������ʱʹ��
BasicBlock* bb;			//һ��basicblock��һ�����뵥���Ļ����飬���Ҷ�Ӧһ����ǩ���� L1��

string funName;		
Value* icmp;			//��Ų��������Ľ�������ڱȽϱ��ʽ
TreeNode* localDec, * stmt_list;	

//for control
BasicBlock* trueBB, * falseBB, * conti, * iterBB;
BasicBlock* retBB, *trueWhile, *falseWhile;
Value* additive;	//��Ӽ������ʽ�Ľ��
Value* mul, *br;	//br��������ת������
Value* call;		//��ź������õķ���ֵ
TreeNode* forGetCallPara;	//Ϊ�˱������������������ʱ����
vector<Type*> functionParams;	//����һ������ʱ����¼�����ı�������
vector<string> functionParamsNames;	//��������Ͷ�Ӧ�Ĳ�����
vector<Value*> callFunParamsLoad;	//���ú���ʱ����Ŵ��ݹ�ȥ�Ĳ���
void getIR(TreeNode* tree);			//����IR�ĺ������ݹ����������IR
stack<BasicBlock*> trueBBforSelect, falseBBforSelect, contiBBForSelect, iterBBforWhile, trueBBforWhile, falseBBforWhile;
						//���if��while����Ƕ�������ջ
function_Info newFunction() {
	function_Info t;		//�������������Ľ�
	return t;
}
qtlocalVar newqtlocalVar() {	//��¼һ����������ڵı�����Ϣ
	qtlocalVar t;
	return t;
}
vector<Value*> args;		//Ϊ������������ռ�ʱ������ʱ����

Value* arrayDec;
Type* arrayParam = ArrayType::get(T32, 0);	//�������ͺ���������ʱ����
Value* arrayTempEle;			//ָ��һ������Ԫ�ص���ʱ����
int arrayDecLength;				//��������ʱ��¼���鳤�ȵ���ʱ����
forFunUsedInfo funUsedInfo;		
char program[100];				//Դ�����ļ���
NodeKind preNodeKind = ErrorK; //
/*-------------------------------------*/
/*        ����Ƕ��          */
int nowptr; //	ָ���������Ƕ���е���һ��
/*-------------------------------------*/
int printTab(map<function_name, function_Info> functions);	//��ӡ���ű���
int main() 
{
	TreeNode* syntaxTree;		//����AST�ĸ�
	strcpy(program, "code.c");
	source = fopen(program, "r");
	if (!source) 
	{
		fprintf(stderr, "Can not open the c\n");
		exit(-1);
	}
	module = new Module(program, context);	//��Ӧ�ڱ�����Դ�ļ�
	listing = stdout;		//ָ����ӡ������Ϣ������̨
	syntaxTree = parse();	//����AST
	
	fprintf(listing, "\nSyntax tree:\n");
	printTree(syntaxTree);
	getIR(syntaxTree);	
	printTab(functions);
	builder.ClearInsertionPoint();
	FILE* stream;
	stream = freopen("main.ll", "w", stdout);
	module->print(outs(), nullptr);			//�����ɵ��м����IR�����main.ll�ļ�
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
			//�����ASTʱ�Ѵ���������ظ�����
			break;

		case VariableDeclarationK:
			cout << "Hi, I am VariableDeclarationK" << endl;
			temps.clear();
			temps = tree->attr.varDecl._var->attr.ID;	//��¼������
			if (position=="global") 
			{
				if (globalVariableAlloca.count(temps) == 0)
				{
					/*GlobalVariable* glo = new GlobalVariable(T32, false, GlobalValue::LinkageTypes::ExternalLinkage);
					glo->setAlignment(10);
					glo->setInitializer(NULL);*/
					//û�ҵ�����ȫ�ֱ�����API
					cout << "line�� " << tree->lineno<<"    �ݲ�֧��ʹ��ȫ�ֱ�����" << endl;
				}
				else {}//ȫ�ֱ����ظ�����
			}
			else
			{
				nowptr = functions[position].localVarStack.size() - 1;   //ָ��������ڵ�Ƕ�ײ���
				if (functions[position].localVarStack[nowptr].localVariableAlloca.count(temps) == 0)
				{
					// ���⣬ջ�������ǿ�
					functions[position].localVarStack[nowptr].localVariableAlloca[temps] = builder.CreateAlloca(T32);//Ϊ��������ռ䲢��¼
					functions[position].localVarStack[nowptr].localVariableLoad[temps] = NULL;			
					/*���ű��¼��Ϣ--------*/
					functions[position].localVarInfo[temps].varName = temps;
					functions[position].localVarInfo[temps].decLineNo = tree->lineno;
					functions[position].localVarInfo[temps].type = MYINT;
					functions[position].localVarInfo[temps].isParam = false;
					/*-----------------------------------------------------*/
				}
				else 
				{
					cout << "line�� " << tree->lineno << " ������"<<temps<<"�ظ����壡" << endl;
				}
			}
			preNodeKind = VariableDeclarationK;
			break;

		case ArrayDeclarationK:
			cout << "Hi, I am ArrayDeclarationK" << endl;
			temps.clear();
			temps = tree->attr.arrDecl._var->attr.ID;		//������
			if (position == "global") 
			{
				if (globalVariableAlloca.count(temps) == 0) 
				{
					//globalVariableAlloca[temps] = builder.CreateAlloca(Type::getArrayElementType(context));
					cout << "line�� " << tree->lineno << "    �ݲ�֧��ʹ��ȫ�ֱ�����" << endl;
				}
				else {}
			}
			else 
			{
				nowptr = functions[position].localVarStack.size() - 1;
				if( functions[position].localVarStack[nowptr].arrayAlloca.count(temps) == 0 )
				{
					arrayDecLength = tree->attr.arrDecl._num->attr.NUM;		//�����С
					myArray = ArrayType::get(T32, arrayDecLength);			
					arrayDec = builder.CreateAlloca(myArray);		//��������ռ�
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
					cout << "line�� " << tree->lineno << " ������" << temps << "�ظ����壡" << endl;
				}
			}
			break;

		case FunctionDeclarationK:
			cout << "Hi, I am FunctionDeclarationK" << endl;
			funName = tree->attr.funcDecl._var->attr.ID;  //������
			functions[funName] = newFunction();			 //���ڼ�¼����ȫ����Ϣ�Ľṹ��
			functions[funName].funcDecLineNo = tree->lineno;
			functionParams.clear();
			functionParamsNames.clear();
			position = funName;
			functions[position].localVarStack.push_back(newqtlocalVar());	//�����壬Ҳ��һ���µĸ������
			getIR(tree->attr.funcDecl.params);			//��ȡ�����Ĳ�����Ϣ���������ͼ�¼��functionParams����������¼��functionParamsNames��
			if (tree->attr.funcDecl.type_spec->attr.TOK == INT) 
			{		//��������ͷ
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
			bb = BasicBlock::Create(context, "", fun);		//�����帴�����
			builder.SetInsertPoint(bb);						//��ʾ�������Ĵ���������һ�����帴�����
			nowptr = functions[position].localVarStack.size() - 1;
			for (int i = 0; i < functionParams.size(); i++) //Ϊ������������ռ�
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
			for (int i = 0; i < args.size(); i++) //�����������ߴ������Ĳ���ֵ��������Ŀռ���
			{
				if (functionParams[i] == T32) 
				{
					builder.CreateStore(args[i],
						functions[position].localVarStack[nowptr].localVariableAlloca[functionParamsNames[i]]);
				}
			}
			getIR(tree->attr.funcDecl.cmpd_stmt);//����������Ĵ���
		
			// out from function dec
			functions[position].localVarStack.pop_back();	//һ������������
			position = "global";		//���˺���������ȫ������
			break;

		case VariableParameterK:
			cout << "Hi, I am VariableParameterK" << endl;
			temps = tree->attr.varParam._var->attr.ID;	//������
			nowptr = functions[position].localVarStack.size() - 1;
			if(functions[position].localVarStack[nowptr].localVariableAlloca.count(temps)==0){
				functionParams.push_back(T32);			//��������
				functionParamsNames.push_back(temps);	//������
				/*=================== symbol table =======================*/
				functions[position].localVarInfo[temps].decLineNo = tree->lineno;
				functions[position].localVarInfo[temps].isParam = true;
				functions[position].localVarInfo[temps].type = MYINT;
				functions[position].localVarInfo[temps].varName = temps;
				/*==============================================*/
			}
			else {
				cout << "line�� " << tree->lineno << " ������" << temps << "�ظ����壡" << endl;
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
				cout << "line�� " << tree->lineno << " ������" << temps << "�ظ����壡" << endl;
			}
			break;

		case CompoundStatementK:
			cout << "Hi, I am CompoundStatementK" << endl;
			functions[position].localVarStack.push_back(newqtlocalVar());//һ���µĸ������
			getIR(tree->attr.cmpdStmt.local_decl);
			getIR(tree->attr.cmpdStmt.stmt_list);
			functions[position].localVarStack.pop_back();	//������������
			break;

		case ExpressionStatementK:
			cout << "Hi, I am ExpressionStatementK" << endl;
			getIR(tree->attr.exprStmt.expr);
			tree->Load = tree->attr.exprStmt.expr->Load;		//���ʽ��ֵ���ۺ����ԣ���¼��������ϣ������ڵ�ʹ��
			break;

		case SelectionStatementK:
			cout << "Hi, I am SelectionStatementK" << endl;
			trueBB = BasicBlock::Create(context, "", fun);//��ǩ��λ��true������������
			trueBBforSelect.push(trueBB);
			falseBB = BasicBlock::Create(context, "", fun);//��ǩ��λ��false������������
			falseBBforSelect.push(falseBB);
			conti = BasicBlock::Create(context, "", fun);//��ǩ��λ��false������֮��ĵ�һ�����
			contiBBForSelect.push(conti);			//����ջ��Ϊ����Ƕ�׶���
			getIR(tree->attr.selectStmt.expr);		//����true����������
			if (tree->attr.selectStmt.expr->nodeKind == ComparisonExpressionK)//�����ǲ�����������ݲ���ֵ�жϣ�������㼴��������
			{
				icmp = tree->attr.selectStmt.expr->Load;
			}
			else 
			{
				icmp = builder.CreateICmpNE(tree->attr.selectStmt.expr->Load, CONST(0));//compare not equal
			}
			trueBB = trueBBforSelect.top(); trueBBforSelect.pop();
			falseBB = falseBBforSelect.top(); 
			br = builder.CreateCondBr(icmp, trueBB, falseBB);//������������ת��true�����죬����false�����죬\
															 ���û��else��䣬��falseBB��conti�൱��ͬһ����ǩ
			builder.SetInsertPoint(trueBB);			//����true�������IR
			getIR(tree->attr.selectStmt.if_stmt);
			if (preNodeKind != ReturnStatementK) //���true����������return��䣬������ת��false������֮���ˣ���Ϊ�����Ѿ�������
			{
				conti = contiBBForSelect.top();
				builder.CreateBr(conti);
			}
			falseBB = falseBBforSelect.top();	
			falseBBforSelect.pop();
			builder.SetInsertPoint(falseBB);
			
			getIR(tree->attr.selectStmt.else_stmt);
			if (preNodeKind != ReturnStatementK)	//ͬ��
			{
				conti = contiBBForSelect.top();
				builder.CreateBr(conti);
			}
			conti = contiBBForSelect.top();  
			contiBBForSelect.pop();
			builder.SetInsertPoint(conti);//����false������֮�������IR
			break;

		case IterationStatementK:
			cout << "Hi, I am IterationStatementK" << endl;
			iterBB = BasicBlock::Create(context, "", fun);//��Ӧѭ���������жϻ�����
			iterBBforWhile.push(iterBB);
			builder.CreateBr(iterBB);//������ʱ��Ҫ����������ѭ�������жϻ����飬����˻�����û����ڣ������޷�����ִ��
			builder.SetInsertPoint(iterBB);
			getIR(tree->attr.iterStmt.expr);
			if (tree->attr.iterStmt.expr->nodeKind == ComparisonExpressionK) //ͬ��
			{
				icmp = tree->attr.iterStmt.expr->Load;
			}
			else 
			{
				icmp = builder.CreateICmpNE(tree->attr.iterStmt.expr->Load, CONST(0));
			}
			trueWhile = BasicBlock::Create(context, "", fun);//trueBBforWhile��falseBBforWhile����ջΪ����Ƕ�׶���
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
				getIR(tree->attr.retStmt.expr);//��ȡҪ���ص�ֵ
				//functions[position].localVarStack[functions[position].localVarStack.size() - 1].localVariableLoad["return"] = tree->attr.retStmt.expr->Load;
				builder.CreateRet(tree->attr.retStmt.expr->Load);//�ۺ�����
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
			if (tree->attr.assignStmt._var->nodeKind == VariableK) //��ֵ�Ǽ򵥱���
			{
				getIR(tree->attr.assignStmt.expr);//��ȡ��ֵ
				bool flag = 0;
				temps = tree->attr.assignStmt._var->attr.ID;//��ֵ������
				for (int i = nowptr; i >= 0; i--) //��Ƕ�׵��ڲ�����Ѱ�ұ����Ķ���λ��
				{
					if (functions[position].localVarStack[i].localVariableAlloca.count(temps) != 0) {//�ҵ���
						builder.CreateStore(builder.CreateLoad(tree->attr.assignStmt.expr->Alloca),
											functions[position].localVarStack[i].localVariableAlloca[temps]);
						functions[position].localVarInfo[temps].useLineNo.push_back(tree->lineno);
						//������ֵ��Ӧ�Ŀռ�
						flag = 1;
						break;
					}
				}
				//�������δ����
				if (flag == 0) {
					cout << "line�� " << tree->lineno << " ������" << temps << "δ���壡" << endl;
				}
			}
			else if (tree->attr.assignStmt._var->nodeKind == ArrayK)	//��ֵ������Ԫ��
			{
				int flag = 0;
				getIR(tree->attr.assignStmt.expr);
				getIR(tree->attr.assignStmt._var->attr.arr.arr_expr);//��ȡ����Ԫ���±�
				temps = tree->attr.assignStmt._var->attr.arr._var->attr.ID;//������
				for (int i = nowptr; i >= 0; i--) {			//��Ƕ�׵��ڲ�����Ѱ�ұ����Ķ���λ��
					if (functions[position].localVarStack[i].arrayAlloca.count(temps) != 0) {
						arrayTempEle = builder.CreateGEP(functions[position].localVarStack[i].arrayAlloca[temps],
							{ CONST(0), tree->attr.assignStmt._var->attr.arr.arr_expr->Load });	//�ҵ���Ԫ�ص�λ��
						builder.CreateStore(tree->attr.assignStmt.expr->Load, arrayTempEle);	//����ֵ���ȥ
						flag = 1;
						functions[position].localVarInfo[temps].useLineNo.push_back(tree->lineno);
						break;
					}
				}
				if (flag == 0) {
					cout << "line�� " << tree->lineno << " ������" << temps << "δ���壡" << endl;
				}
			}
			break;

		case ComparisonExpressionK:
			cout << "Hi, I am ComparisonExpressionK" << endl;
			getIR(tree->attr.cmpExpr.lexpr);//��ȡ�Ƚ�ʽ����ֵ
			getIR(tree->attr.cmpExpr.rexpr);//��ֵ
			//tree->attr.cmpExpr.lexpr->Load = builder.CreateLoad(tree->attr.cmpExpr.lexpr->Alloca);
			//tree->attr.cmpExpr.rexpr->Load = builder.CreateLoad(tree->attr.cmpExpr.rexpr->Alloca);
			switch (tree->attr.cmpExpr.op->attr.TOK)	//�����ȽϽ��
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
			tree->Load = icmp;	//�������ϣ������ڵ�ʹ��
			break;

		case AdditiveExpressionK:
			cout << "Hi, I am AdditiveExpressionK" << endl;
			getIR(tree->attr.addExpr.lexpr);	//���ʽ��������
			getIR(tree->attr.addExpr.rexpr);	//���ʽ��������
			switch (tree->attr.addExpr.op->attr.TOK)//����������
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


		case MultiplicativeExpressionK:		//ͬ�Ӽ����ʽ
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
			temps = tree->attr.ID;	//����ı���һ������Ϊ��ֵʹ�ã���ʹ������ֵ������ʹ�����Ŀռ��ַ
			int flag = 0;
			nowptr = functions[position].localVarStack.size() - 1;
			for (int i = nowptr; i >= 0; i--) 
			{
				if (functions[position].localVarStack[i].localVariableAlloca.count(temps) != 0) 
				{
					if (tree->Alloca == NULL)
					{
						tree->Alloca = builder.CreateAlloca(T32);
					}//LLVM IR��֧�ֽ�ͬһλ�õ�ֵ��ֵ����λ�ã�������ȡ�����ŵ���ĵط����ٷŻ�ȥ������ᱨ��
					builder.CreateStore(builder.CreateLoad(functions[position].localVarStack[i].localVariableAlloca[temps]), tree->Alloca);
					tree->Load = builder.CreateLoad(tree->Alloca);//��ֵȡ�����ŵ����ϣ������ڵ�ʹ��
					/*=========================  for symbol table  =============================*/
					functions[position].localVarInfo[temps].useLineNo.push_back(tree->lineno);
					/*-------------------------------------------------------------------------------*/
					flag = 1;
					break;
				}
			}
			if (flag == 0) {
				cout << "line�� " << tree->lineno << " ������" << temps << "δ���壡" << endl;
			}
			break;
		}
		case ArrayK:		//����ͬ��
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
				cout << "line�� " << tree->lineno << " ������" << temps << "δ���壡" << endl;
			}
			break;
		}

		case CallK:
			cout << "Hi, I am CallK" << endl;
			temps = tree->attr.call._var->attr.ID;//Ҫ���õĺ�����
			if (functions.count(temps) == 0) 
			{
				cout << "ERROR! ���� \"" << temps << "\" δ���壡" << endl;
			}
			else 
			{
				callFun = functions[temps].function;
				callFunParamsLoad.clear();
				getIR(tree->attr.call.expr_list);//��������ǵ�ֵ
				forGetCallPara = tree->attr.call.expr_list;
				while (forGetCallPara)//��¼���в�����ֵ
				{
					callFunParamsLoad.push_back(forGetCallPara->Load);
					forGetCallPara = forGetCallPara->sibling;
				}
				//cout << callFunParamsLoad.size() << endl;
				call = builder.CreateCall(callFun, callFunParamsLoad);//��������
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
			tree->Load = builder.CreateLoad(tree->Alloca);//������ֵ�ŵ����Ϲ����ڵ����
			break;

		case TokenTypeK:
			cout << "Hi, I am TokenTypeK" << endl;
			break;

		default:
			;
		}
	}
	
}
