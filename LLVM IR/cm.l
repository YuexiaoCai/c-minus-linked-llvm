

%{		/*指明将要引用的头文件，这部分将被照搬到生成的lex.yy.c文件中*/
	#include "globals_util.h"
	#include "AST_util.h"
	char tokenString[MAXTOKENLEN+2];
%}

%x COMMENT
		/*辅助定义正规表达式*/
		/*引用别的已定义的符号使使用花括号*/
		/*红色警报：不能在正规表达式后面写注释，无语了，调了好久才从经验中发现*/
letter  		[a-zA-Z]
digit   		[0-9]
number  		[1-9]{digit}*	
identifier 		{letter}+		
newline			\n
whitespace		[ \t]+


%%
"/*"                { BEGIN(COMMENT); }		
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>.          { /* skip comments */ }
<COMMENT>{newline}  { lineno++; }
<COMMENT><<EOF>>    { BEGIN(INITIAL); return ERROR; }
"*/"                { return ERROR; }
<<EOF>>             { return ENDFILE; }
	/*规则*/
({letter}+{digit}|{digit}+{letter})({letter}|{digit})* { return SelfDecError; }
"else"              { return ELSE; }
"if"                { return IF; }
"int"               { return INT; }
"return"            { return RETURN; }
"void"              { return VOID; }
"while"             { return WHILE; }
"+"                 { return PLUS; }
"-"                 { return MINUS; }
"*"                 { return TIMES; }
"/"                 { return OVER; }
"<"                 { return LT; }
"<="                { return LE; }
">"                 { return GT; }
">="                { return GE; }
"=="                { return EQ; }
"!="                { return NE; }
"="                 { return ASSIGN; }
";"                 { return SEMI; }
","                 { return COMMA; }
"("                 { return LPAREN; }
")"                 { return RPAREN; }
"["                 { return LBRACK; }
"]"                 { return RBRACK; }
"{"                 { return LBRACE; }
"}"                 { return RBRACE; }
{number}            { return NUM; }
{identifier}        { return ID; }
{newline}           { lineno++; }
{whitespace}        { /* skip whitespace */ }


%%

TokenType getToken(void)
{
  static int firstTime = TRUE;
  TokenType currentToken;       /* TokenType is Int Type*/
  if (firstTime)
	{
	  firstTime = FALSE;
	  lineno++;
	  yyin = source;			/*这两个只需要指定一次*/
	  yyout = listing;
	}
  currentToken = yylex();		/*.y文件中定义的函数，最终在Cminus.tab.c中*/
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan)
  {
      fprintf(listing,"\t%d\t\t\t",lineno);
      printToken(currentToken,tokenString);
  }
  return currentToken;
}