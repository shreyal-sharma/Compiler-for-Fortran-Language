#include "parser.h"
#include <map>
#include "lex.h"

map<string, bool> defVar;
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

void pushBackWrapper(LexItem& token) {
    Parser::PushBackToken(token);
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
        pushBackWrapper(t);
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
    // Parse program identifier

    token = Parser::GetNextToken(in, line);
    if (token != IDENT) {
        ParseError(line, "Missing IDENT");
        return false;
    }
    LexItem t = Parser::GetNextToken(in, line);
    pushBackWrapper(t);

    // Parse declarations

    while (t == INTEGER || t == REAL || t == CHARACTER) {
        Decl(in, line);
        if (t == CHARACTER && charlength != "") {
            cout << "Definition of Strings with length of " << charlength << " in declaration statement." << endl;
            charlength = "";
        }
        t = Parser::GetNextToken(in, line);
        pushBackWrapper(t);
    }

    // Parse statements

    while (Stmt(in, line));

    // Check for END PROGRAM keyword
    token = Parser::GetNextToken(in, line);

    if (token != END) {
        ParseError(line, "Missing END statement in program");
        return false;
    }

    // Check for PROGRAM keyword

    token = Parser::GetNextToken(in, line);
    if (token != PROGRAM) {
        ParseError(line, "Missing END PROGRAM keyword");
        return false;
    }
    // Check if program identifiers match

    token = Parser::GetNextToken(in, line);
    if (token != IDENT) {
        ParseError(line, "Missing IDENT");
        return false;
    }
    cout << "(DONE)" << endl;
    return true;
}

//Decl ::= Type :: VarList
bool Decl(istream& in, int& line) {
    // Parse the type

    if (!Type(in, line)) {
        return false;
    }

    // Check for the :: separator
    LexItem token = Parser::GetNextToken(in, line);
    if (token != DCOLON) {
        pushBackWrapper(token);
        ParseError(line, "Missing Double Colons");
        return false;
    }    return VarList(in, line);

}

//Type ::= INTEGER | REAL | CHARARACTER [(LEN = ICONST)]
bool Type(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    //check if int or real or char
    if (token != INTEGER && token != REAL && token != CHARACTER) {
        pushBackWrapper(token);
        return false;
    }
    // if char
    // check if length given
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
            // push back the token, there is not optional length
            pushBackWrapper(token);
        }
    }
    return true;
}

//VarList ::= Var [= Expr] {, Var [= Expr]}
bool VarList(istream& in, int& line) {
    // Parse the first variable
    if (!Var(in, line)) {
        return false;
    }
    LexItem varToken = Parser::GetNextToken(in, line);
    if (defVar.find(varToken.GetLexeme()) == defVar.end()) {
        defVar[varToken.GetLexeme()] = true;
    }
    else {
        ParseError(line, "var declared already");
        return false;
    }
    LexItem token = Parser::GetNextToken(in, line);
    if (token == ASSOP) {
        if (!Expr(in, line)) {
            return false;
        }

        // print statement for variable initalization
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
            ParseError(line, "var declared already");
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
        pushBackWrapper(token);
        return true;
    }
    else {
        pushBackWrapper(token);
        return false;
    }
}

//Stmt ::= AssigStmt | BlockIfStmt | PrintStmt | SimpleIfStmt

bool Stmt(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token == DCOLON) {
        token = Parser::GetNextToken(in, line);
    }
    if (token == IDENT) {
        pushBackWrapper(token);
        return AssignStmt(in, line);
    }
    else if (token == IF) {
        // Parse both SimpleIfStmt and BlockIfStmt
        pushBackWrapper(token);
        return BlockIfStmt(in, line);
    }
    else if (token == PRINT) {
        return PrintStmt(in, line);
    }
    else if (token == ELSE || token == PROGRAM || token == END) {
        pushBackWrapper(token);
        return false;
    }
    else {
        pushBackWrapper(token);
        return SimpleIfStmt(in, line);
    }
}

//SimpleStmt ::= AssigStmt | PrintStmt
bool SimpleStmt(istream& in, int& line) {
    bool correct = true;
    //  string errMsg = "";

    LexItem token = Parser::GetNextToken(in, line);
    if (token == IDENT) {
        pushBackWrapper(token);
        correct= AssignStmt(in, line);
    }
    else if (token == PRINT) {
        correct= PrintStmt(in, line);
    }

    // Unsuccessful parsing
    // return false.

    if(!correct) {

        return false;
    }
    return true;
}

//BlockIfStmt ::= IF (RelExpr) THEN {Stmt} [ELSE {Stmt}] END IF
bool BlockIfStmt(istream& in, int& line) {
    // Parse the IF keyword
    LexItem token = Parser::GetNextToken(in, line);
    if (token != IF) {
        pushBackWrapper(token);
        return false;
    }
    blockifstatment++;

    // Parse the opening parenthesis
    token = Parser::GetNextToken(in, line);
    if (token != LPAREN) {
        return false;
    }

    // Parse the relational expression
    if (!RelExpr(in, line)) {
        return false;
    }

    // Parse the closing parenthesis
    token = Parser::GetNextToken(in, line);
    if (token != RPAREN) {
        return false;
    }
    // Parse the THEN keyword

    token = Parser::GetNextToken(in, line);
    if (token != THEN) {
        if (token == PRINT) {
            cout << "Print statement in a Simple If statement." << endl;
        }
        pushBackWrapper(token);
        return SimpleStmt(in, line);
    }

    // Parse the block of statements within the IF block
    while (Stmt(in, line));

    token = Parser::GetNextToken(in, line);
    if (token != ELSE && token != END) {
        ParseError(line, "Missing END in if");
        return false;
    }
    if (token == ELSE) {
        while (Stmt(in, line));
        token = Parser::GetNextToken(in, line);
        if (token != END) {
            ParseError(line, "Missing END in if");
            return false;
        }

    }

    token = Parser::GetNextToken(in, line);
    if (token != IF) {
        ParseError(line, "Missing IF");
        return false;
    }
    cout << "End of Block If statement at nesting level " << blockifstatment << endl;
    blockifstatment--;
    return true;
}

//SimpleIfStmt ::= IF (RelExpr) SimpleStmt
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
        ParseError(line, "Undefined var");
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
        ParseError(line, "Missing IDENT");
        return false;
    }
    pushBackWrapper(token);
    return true;
}

bool RelExpr(istream& in, int& line) {
    if (!Expr(in, line)) {
        ParseError(line, "Missing expr");
        return false;
    }
    LexItem token = Parser::GetNextToken(in, line);
    if (token == EQ || token == LTHAN || token == GTHAN) {
        return Expr(in, line);
    }
    pushBackWrapper(token);
    return true;
}

bool Expr(istream& in, int& line) {
    if (!MultExpr(in, line)) {
        ParseError(line, "wrong expr");
        return false;
    }

    LexItem token = Parser::GetNextToken(in, line);
    while (token == PLUS || token == MINUS || token == CAT) {
        if (!MultExpr(in, line)) {
            ParseError(line, "wrong expr");
            return false;
        }
        token = Parser::GetNextToken(in, line);
    }
    pushBackWrapper(token);
    return true;
}

bool MultExpr(istream& in, int& line) {
    if (!TermExpr(in, line)) {
        ParseError(line, "wrong expr");
        return false;
    }

    LexItem token = Parser::GetNextToken(in, line);
    while (token == MULT || token == DIV) {
        if (!TermExpr(in, line)) {
            ParseError(line, "wrong expr");
            return false;
        }
        token = Parser::GetNextToken(in, line);
    }
    pushBackWrapper(token);
    return true;
}

bool TermExpr(istream& in, int& line) {
    if (!SFactor(in, line)) {
        ParseError(line, "wrong term");
        return false;
    }
    LexItem token = Parser::GetNextToken(in, line);
    while (token == POW) {
        if (!SFactor(in, line)) {
            ParseError(line, "wrong factor");
            return false;
        }
        token = Parser::GetNextToken(in, line);
    }
    pushBackWrapper(token);
    return true;
}

bool SFactor(istream& in, int& line) {
    LexItem token = Parser::GetNextToken(in, line);
    if (token == PLUS || token == MINUS) {
        return Factor(in, line, (token == PLUS) ? -1 : 1); //sign based on token
    }
    else {
        pushBackWrapper(token);
        return Factor(in, line, 1); // positive for other tokens
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
                ParseError(line, "undefined var");
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
            ParseError(line, "Missing ')'");
            return false;
        }
        return true;
    }
    else {
        return false;
    }
}