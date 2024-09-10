#include "parser.h"
#include <map>
#include "lex.h"

map<string, bool> defVar;
map<string, Token> SymTable;
static string charlength = "";
static int blockifstatment = 0;

namespace Parser {
    bool pushed_back = false;
    LexItem	pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem& t) {
        if (pushed_back) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)

{
    ++error_count;
    cout << line << ": " << msg << endl;
}

bool IdentList(istream& in, int& line);

//PrintStmt:= PRINT *, ExpreList
bool PrintStmt(istream& in, int& line) {
    LexItem t;
    t = Parser::GetNextToken(in, line);
    if (t != DEF) {
        ParseError(line, "Print statement syntax error.");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != COMMA) {
        ParseError(line, "Missing Comma.");
        return false;
    }
    bool e = ExprList(in, line);
    if (!e) {
        ParseError(line, "Missing expression after Print Statement");
        return false;
    }
    return e;
}//End of PrintStmt


//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
    bool status = false;
    status = Expr(in, line);
    if (!status) {
        ParseError(line, "Missing Expression");
        return false;
    }
    LexItem t = Parser::GetNextToken(in, line);

    if (t == COMMA) {
        status = ExprList(in, line);
    }
    else if (t.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    else {
        Parser::PushBackToken(t);
        return true;
    }
    return status;
}//End of ExprList

//Prog ::= PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
bool Prog(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token != PROGRAM) {
        ParseError(line, "Missing PROGRAM");
        return false;
    }
    token = Parser::GetNextToken(in, line);
    if (token != IDENT) {
        ParseError(line, "Missing IDENTIFIER");
        return false;
    }
    LexItem t = Parser::GetNextToken(in, line);
    Parser::PushBackToken(t);
    while (t == INTEGER || t == REAL || t == CHARACTER) {
        Decl(in, line);
        if (t == CHARACTER && charlength != "") {
            cout << "Definition of Strings with length  " << charlength << " in declaration statement." << endl;
            charlength = "";
        }
        t = Parser::GetNextToken(in, line);
        Parser::PushBackToken(t);
    }
    while (Stmt(in, line));

    token = Parser::GetNextToken(in, line);


    if (token != END) {
        ParseError(line, "Missing END of program");
        return false;
    }

    token = Parser::GetNextToken(in, line);
    if (token != PROGRAM) {
        ParseError(line, "Missing end PROGRAM");
        return false;
    }
    token = Parser::GetNextToken(in, line);
    if (token != IDENT) {
        ParseError(line, "Missing end IDENT");
        return false;
    }
    cout << "(DONE)" << endl;
    return true;
}
bool Decl(istream& in, int& line) {
    if (!Type(in, line)) {
        return false;
    }
    LexItem token = Parser::GetNextToken(in, line);
    if (token != DCOLON) {
        Parser::PushBackToken(token);
        ParseError(line, "Missing DCOLON");
        return false;
    }
    return VarList(in, line);
}
bool Type(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token != INTEGER && token != REAL && token != CHARACTER) {
        Parser::PushBackToken(token);
        return false;
    }
    if (token == CHARACTER) {
        token = Parser::GetNextToken(in, line);
        if (token == LPAREN) {
            token = Parser::GetNextToken(in, line);
            if (token != LEN) {
                return false;
            }
            token = Parser::GetNextToken(in, line);
            if (token != ASSOP) {
                return false;
            }
            token = Parser::GetNextToken(in, line);
            if (token != ICONST) {
                return false;
            }
            charlength = token.GetLexeme();
            token = Parser::GetNextToken(in, line);
            if (token != RPAREN) {
                return false;
            }
        }
        else {
            Parser::PushBackToken(token);
        }
    }
    return true;
}

bool VarList(istream& in, int& line) {
    if (!Var(in, line)) {
        return false;
    }
    LexItem varToken = Parser::GetNextToken(in, line);
    if (defVar.find(varToken.GetLexeme()) == defVar.end()) {
        defVar[varToken.GetLexeme()] = true;
    }
    else {
        ParseError(line, "Variable already declared");
        return false;
    }
    LexItem token = Parser::GetNextToken(in, line);
    if (token == ASSOP) {
        if (!Expr(in, line)) {
            return false;
        }
        cout << "Initialization of the variable " << varToken.GetLexeme() << " in the declaration statement." << endl;
        token = Parser::GetNextToken(in, line);
    }
    while (token == COMMA) {
        if (!Var(in, line)) {
            return false;
        }
        varToken = Parser::GetNextToken(in, line);
        if (defVar.find(varToken.GetLexeme()) == defVar.end()) {
            defVar[varToken.GetLexeme()] = true;
        }
        else {
            ParseError(line, "Variable already declared");
            return false;
        }
        token = Parser::GetNextToken(in, line);
        if (token == ASSOP) {
            if (!Expr(in, line)) {
                return false;
            }
            cout << "Initialization of the variable " << varToken.GetLexeme() << " in the declaration statement." << endl;
            token = Parser::GetNextToken(in, line);
        }
    }

    if (token != END && token != PROGRAM) {
        Parser::PushBackToken(token);
        return true;
    }
    else {
        Parser::PushBackToken(token);
        return false;
    }
}

bool Stmt(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token == DCOLON) {
        token = Parser::GetNextToken(in, line);
    }
    if (token == IDENT) {
        Parser::PushBackToken(token);
        return AssignStmt(in, line);
    }
    else if (token == IF) {
        Parser::PushBackToken(token);
        return BlockIfStmt(in, line);
    }
    else if (token == PRINT) {
        return PrintStmt(in, line);
    }
    else if (token == ELSE || token == PROGRAM || token == END) {
        Parser::PushBackToken(token);
        return false;
    }
    else {
        Parser::PushBackToken(token);
        return SimpleIfStmt(in, line);
    }
}

bool SimpleStmt(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token == IDENT) {
        Parser::PushBackToken(token);
        return AssignStmt(in, line);
    }
    else if (token == PRINT) {
        return PrintStmt(in, line);
    }
    else {
        return false;
    }
}

bool BlockIfStmt(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token != IF) {
        Parser::PushBackToken(token);
        return false;
    }
    blockifstatment++;
    token = Parser::GetNextToken(in, line);
    if (token != LPAREN) {
        return false;
    }
    if (!RelExpr(in, line)) {
        return false;
    }

    token = Parser::GetNextToken(in, line);
    if (token != RPAREN) {
        return false;
    }

    token = Parser::GetNextToken(in, line);
    if (token != THEN) {
        if (token == PRINT) {
            cout << "Print statement in a Simple If statement." << endl;
        }
        Parser::PushBackToken(token);
        return SimpleStmt(in, line);
    }

    while (Stmt(in, line));

    token = Parser::GetNextToken(in, line);
    if (token != ELSE && token != END) {
        ParseError(line, "Missing END after IF block");
        return false;
    }
    if (token == ELSE) {
        while (Stmt(in, line));
        token = Parser::GetNextToken(in, line);
        if (token != END) {
            ParseError(line, "Missing END after IF block");
            return false;
        }

    }

    token = Parser::GetNextToken(in, line);
    if (token != IF) {
        ParseError(line, "Missing IF after END");
        return false;
    }
    cout << "End of Block If statement at nesting level " << blockifstatment << endl;
    blockifstatment--;
    return true;
}

bool SimpleIfStmt(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token != IF) {
        return false;
    }

    token = Parser::GetNextToken(in, line);
    if (token != LPAREN) {
        return false;
    }

    if (!RelExpr(in, line)) {
        return false;
    }

    token = Parser::GetNextToken(in, line);
    if (token != RPAREN) {
        return false;
    }
    return SimpleStmt(in, line);
}

bool AssignStmt(istream& in, int& line) {
    if (!Var(in, line)) {
        return false;
    }

    LexItem token = Parser::GetNextToken(in, line);

    if (defVar.find(token.GetLexeme()) == defVar.end()) {
        ParseError(line, "Using Undefined Variable");
        return false;
    }

    token = Parser::GetNextToken(in, line);
    if (token != ASSOP) {
        return false;
    }
    return Expr(in, line);
}

bool Var(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token != IDENT) {
        ParseError(line, "Expected an identifier");
        return false;
    }
    Parser::PushBackToken(token);
    return true;
}

bool RelExpr(istream& in, int& line) {
    if (!Expr(in, line)) {
        ParseError(line, "Missing expression in relational expression");
        return false;
    }
    LexItem token = Parser::GetNextToken(in, line);
    if (token == EQ || token == LTHAN || token == GTHAN) {
        return Expr(in, line);
    }
    Parser::PushBackToken(token);
    return true;
}

bool Expr(istream& in, int& line) {
    if (!MultExpr(in, line)) {
        ParseError(line, "Invalid expression");
        return false;
    }

    LexItem token = Parser::GetNextToken(in, line);
    while (token == PLUS || token == MINUS || token == CAT) {
        if (!MultExpr(in, line)) {
            ParseError(line, "Invalid expression after arithmetic operator");
            return false;
        }
        token = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(token);
    return true;
}

bool MultExpr(istream& in, int& line) {
    if (!TermExpr(in, line)) {
        ParseError(line, "Invalid term in multiplication/division expression");
        return false;
    }

    LexItem token = Parser::GetNextToken(in, line);
    while (token == MULT || token == DIV) {
        if (!TermExpr(in, line)) {
            ParseError(line, "Invalid term after multiplication/division operator");
            return false;
        }
        token = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(token);
    return true;
}

bool TermExpr(istream& in, int& line) {
    if (!SFactor(in, line)) {
        ParseError(line, "Invalid factor in term expression");
        return false;
    }
    LexItem token = Parser::GetNextToken(in, line);
    while (token == POW) {
        if (!SFactor(in, line)) {
            ParseError(line, "Invalid factor after power operator");
            return false;
        }
        token = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(token);
    return true;
}

bool SFactor(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token == PLUS || token == MINUS) {
        return Factor(in, line, 1);
    }
    else {
        Parser::PushBackToken(token);
        return Factor(in, line, 1);
    }
}

bool Factor(istream& in, int& line, int sign) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token == IDENT || token == ICONST || token == RCONST || token == SCONST) {
        if (token == IDENT) {
            if (!(defVar.find(token.GetLexeme()) == defVar.end())) {
                return true;
            }
            else {
                ParseError(line, "Using undefined variable");
                return false;
            }
        }
        else {
            return true;
        }

    }
    else if (token == LPAREN) {
        if (!Expr(in, line)) {
            return false;
        }
        token = Parser::GetNextToken(in, line);
        if (token != RPAREN) {
            cout << token << endl;
            ParseError(line, "Missing closing parenthesis ')'");
            return false;
        }
        return true;
    }
    else {
        cout << "in factor: " << token << endl;
        Parser::PushBackToken(token);
        ParseError(line, "Invalid factor");
        return false;
    }
}