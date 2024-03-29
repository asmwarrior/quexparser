#ifndef STATEMENTSOLVER_H
#define STATEMENTSOLVER_H

#include "tokenizer.h"
#include "symbol.h"
class Piece
{
public:
    Token name;
    std::vector<Token> signature;
    Token oper;
};

class StatementSolver
{
public:
    StatementSolver(cc_string &statement,SymbolTree* tree);
    void Dump();
    void SetInitialSearchScope(SymbolIdxSet &scope);
    int  SuggestionList(SymbolIdxSet &result);


private:
    std::vector<Piece> m_PieceQueue;
    Tokenizer          m_Tokenizer;
    SymbolTree         *m_SymbolTree;
};


#endif //STATEMENTSOLVER_H
