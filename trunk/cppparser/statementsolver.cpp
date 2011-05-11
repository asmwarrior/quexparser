#include "statementsolver.h"



StatementSolver::StatementSolver(cc_string &statement,SymbolTree* tree)
{
    m_Tokenizer.InitFromBuffer(statement);
    m_SymbolTree = tree;
    Token tok;
    vector<Token> toks;
    while(true)
    {
        m_Tokenizer.FetchToken(&tok);
        cout<<tok<<endl;
        toks.push_back(tok);
        if(tok== TKN_TERMINATION)
            break;
    }

    for(int i = 0;i<toks.size()-1;)
    {
        Piece pie;
        pie.name = toks[i];
        i++;
        if(toks[i]==TKN_L_PAREN)
        {
            //pie.signature.push_back(toks[i]);
            i++;
            while(true)
            {
                if(toks[i]!=TKN_R_PAREN)
                {
                    pie.signature.push_back(toks[i]);
                    i++;
                }
                else
                {
                    i++;
                    break;
                }
            }
        }

        if(toks[i]==TKN_DOT || toks[i]==TKN_ARROW || toks[i]==TKN_DOUBLE_COLON)
        {
            pie.oper = toks[i];
            i++;
        }
        m_PieceQueue.push_back(pie);
    }

    Dump();

}
void StatementSolver::Dump()
{
    cout<<"StatementSolver::Dump()\n";
    for(int i = 0;i<m_PieceQueue.size();i++)
    {
        cout<<m_PieceQueue[i].name;
        for(int j = 0;j<m_PieceQueue[i].signature.size();j++)
            cout<<m_PieceQueue[i].signature[j];
        cout<<m_PieceQueue[i].oper;
        cout<<endl;

    }
}
void StatementSolver::SetInitialSearchScope(SymbolIdxSet &scope)
{

}
int  StatementSolver::SuggestionList(SymbolIdxSet &result)
{

}

