/*
 * parser(SP24).h
 * Programming Assignment 2
 * Spring 2024
*/

#ifndef PARSE_H_
#define PARSE_H_

#include <iostream>

using namespace std;

#include "lex.h"



extern bool Prog(istream& in, int& line); //me
extern bool Decl(istream& in, int& line); //me
extern bool Type(istream& in, int& line); //me
extern bool VarList(istream& in, int& line); //me
extern bool Stmt(istream& in, int& line); //me
extern bool SimpleStmt(istream& in, int& line); //me
extern bool PrintStmt(istream& in, int& line); //done
extern bool BlockIfStmt(istream& in, int& line);
extern bool SimpleIfStmt(istream& in, int& line);
extern bool AssignStmt(istream& in, int& line); //me
extern bool Var(istream& in, int& line); //done
extern bool ExprList(istream& in, int& line); //done
extern bool RelExpr(istream& in, int& line); //me
extern bool Expr(istream& in, int& line); //me
extern bool MultExpr(istream& in, int& line);//me
extern bool TermExpr(istream& in, int& line); //me
extern bool SFactor(istream& in, int& line);//me
extern bool Factor(istream& in, int& line, int sign); //me
extern int ErrCount(); //done

#endif /* PARSE_H_ */
