%{
#include "globals_util.h"
#include "AST_util.h"

#define YYSTYPE treeNode*

static int yylex(void);/*声明用户自定义函数*/
int yyerror(char *);
%}

%token MINIMUM_TOKEN
%token ID NUM
%token ELSE IF INT RETURN VOID WHILE
%token PLUS MINUS TIMES OVER
%token LT LE GT GE EQ NE
%token ASSIGN SEMI COMMA
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE
%token ENDFILE SelfDecError
%token MAXIMUM_TOKEN

%nonassoc LOWER_ELSE
%nonassoc ELSE

%start program  /* 指定开始符号，必须有*/
%%
program
        : declaration_list
          { savedTree = $1; }
        ;

declaration_list
        : declaration_list declaration
          { $$ = addSib($1, $2); }
        | declaration
          { $$ = $1; }
        ;

declaration
        : var_declaration
          { $$ = $1; }
        | fun_declaration
          { $$ = $1; }
        ;

var_declaration
        : type_specifier _id SEMI
          { $$ = newVarDecTN($1, $2); }
        | type_specifier _id LBRACK _num RBRACK SEMI
          { $$ = newArrayDecTN($1, $2, $4); }
        ;

type_specifier
        : INT
          { $$ = newTokenTypeTN(INT); }
        | VOID
          { $$ = newTokenTypeTN(VOID); }

fun_declaration
        : type_specifier _id LPAREN params RPAREN compound_stmt
          { $$ = newFunDecTN($1, $2, $4, $6); }
        ;

params
        : param_list
          { $$ = $1; }
        | VOID
          { $$ = NULL; }
        ;

param_list
        : param_list COMMA param
          { $$ = addSib($1, $3); }
        | param
          { $$ = $1; }
        ;

param
        : type_specifier _id
          { $$ = newVarParamTN($1, $2); }
        | type_specifier _id LBRACK RBRACK
          { $$ = newArrayParamTN($1, $2); }
        ;

compound_stmt
        : LBRACE local_declarations statement_list RBRACE
          { $$ = newCompStmtTN($2, $3); }
        ;

local_declarations
        : local_declarations var_declaration
          { $$ = addSib($1, $2); }
        | /* empty */
          { $$ = NULL; }
        ;

statement_list
        : statement_list statement
          { $$ = addSib($1, $2); }
        | /* empty */
          { $$ = NULL; }
        ;

statement
        : expression_stmt
          { $$ = $1; }
        | compound_stmt
          { $$ = $1; }
        | selection_stmt
          { $$ = $1; }
        | iteration_stmt
          { $$ = $1; }
        | return_stmt
          { $$ = $1; }
        ;

expression_stmt
        : expression SEMI
          { $$ = newExprStmtTN($1); }
        | SEMI
          { $$ = NULL; }
        ;

selection_stmt
        : IF LPAREN expression RPAREN statement
          { $$ = newSelectStmtTN($3, $5, NULL); }
          %prec LOWER_ELSE
        | IF LPAREN expression RPAREN statement ELSE statement
          { $$ = newSelectStmtTN($3, $5, $7); }
        ;

iteration_stmt
        : WHILE LPAREN expression RPAREN statement
          { $$ = newIterStmtTN($3, $5); }
        ;

return_stmt
        : RETURN SEMI
          { $$ = newRetStmtTN(NULL); }
        | RETURN expression SEMI
          { $$ = newRetStmtTN($2); }
        ;

expression
        : var ASSIGN expression
         { $$ = newAssignExprTN($1, $3); }
        | simple_expression
         { $$ = $1; }
        ;

var
        : _id
          { $$ = $1; }
        | _id LBRACK expression RBRACK
          { $$ = newArrayTN($1, $3); }
        ;

simple_expression
        : additive_expression relop additive_expression
          { $$ = newCmpExprTN($1, $2, $3); }
        | additive_expression
          { $$ = $1; }
        ;

relop
        : LT
          { $$ = newTokenTypeTN(LT); }
        | LE
          { $$ = newTokenTypeTN(LE); }
        | GT
          { $$ = newTokenTypeTN(GT); }
        | GE
          { $$ = newTokenTypeTN(GE); }
        | EQ
          { $$ = newTokenTypeTN(EQ); }
        | NE
          { $$ = newTokenTypeTN(NE); }
        ;

additive_expression
        : additive_expression addop term
          { $$ = newAddExprTN($1, $2, $3); }
        | term
          { $$ = $1; }
        ;

addop
        : PLUS
          { $$ = newTokenTypeTN(PLUS); }
        | MINUS
          { $$ = newTokenTypeTN(MINUS); }

term
        : term mulop factor
          { $$ = newMulExprTN($1, $2, $3); }
        | factor
          { $$ = $1; }
        ;

mulop
        : TIMES
          { $$ = newTokenTypeTN(TIMES); }
        | OVER
          { $$ = newTokenTypeTN(OVER); }
        ;

factor
        : LPAREN expression RPAREN
          { $$ = $2; }
        | var
          { $$ = $1; }
        | call
          { $$ = $1; }
        | _num
          { $$ = $1; }
        ;

call
        : _id LPAREN args RPAREN
          { $$ = newCallTN($1, $3); }
        ;

args
        : arg_list
          { $$ = $1; }
        | /* empty */
          { $$ = NULL; }
        ;

arg_list
        : arg_list COMMA expression
          { $$ = addSib($1, $3); }
        | expression
          { $$ = $1; }
        ;

_id
        : ID
          { $$ = newVarTN(tokenString); }
        ;

_num
        : NUM
          { $$ = newConstantTN(tokenString); }
        ;



%%

int yyerror(char * message)
{
  fprintf(listing, "Syntax error at line %d: %s\n", lineno, message);
  fprintf(listing, "Current token: ");
  printToken(yychar,tokenString);
  myError = TRUE;
  return 0;
}

static int yylex(void)
{
  TokenType tok = getToken();
  if (tok == ENDFILE) return 0;
  if (tok == SelfDecError)
    {
      fprintf(listing,
              "Lexical analyze error at line %d\n",
              lineno);
      fprintf(listing,
              "Current token: %s",
              tokenString);
      myError = TRUE;
      return 0;
    }
  return tok;
}