#ifndef PREPROCESSOR_H_INCLUDED
#define PREPROCESSOR_H_INCLUDED

#include<list>
#include<stack>
#include<vector>
#include"tokenizer.h"

class MacroDefine
{
public:
    MacroDefine():m_IsFunctionLike(false),m_FileIndex(-1){};
    void Dump()
    {
        std::cout<<"NAME:"<<m_Name.get_string()<<std::endl;
        if(m_IsFunctionLike)
        {
            std::cout<<"ARG:"<<std::endl;
            for(size_t i=0;i<m_Arguments.size();i++)
                std::cout<<m_Arguments[i].get_string()<<std::endl;
        }
        std::cout<<"VALUE:"<<std::endl;
        for(size_t i=0;i<m_DefineValue.size();i++)
            std::cout<<m_DefineValue[i].get_string()<<std::endl;
    }

    Token m_Name;
    bool m_IsFunctionLike;
    std::vector<Token> m_Arguments;
    std::vector<Token> m_DefineValue;
    int  m_FileIndex;
};

typedef std::map<std::string,MacroDefine> MacroTable;


class BranchEntry
{
public:
    BranchEntry():m_Value(false),m_Line(-1){};

    bool m_Value;
    int  m_Line;
};

typedef std::stack<BranchEntry> BranchStack;


class Preprocessor
{
public:
    Preprocessor();
    ~Preprocessor();

    void  LoadFile(cc_string filename);
    void  Preprocess();

    void  RunTest();
    void  RunTestPerformance();
    void  DumpMacroTable();
    void  DumpTokenList();

    // Token manipulation
    Token*  GetToken();
    Token*  CurrentToken();
    Token*  PeekToken(int step = 1);
    void  UngetToken();

    bool CheckMacroExist(std::string key);

private:
    bool ConstExpressionValue();
    void AddMacroDefinition();
    bool ConstExpressionExpansion(std::vector<Token> & exp);
    void ReadMacroActualArgument(std::vector<std::vector<Token> > &arg, int argNum);
    void MacroExpansion(std::vector<Token> &expend,MacroDefine &def, std::vector<std::vector<Token> > &arg);

    void HandleIf();
    void HandleElif();
    void HandleIfdef();
    void HandleIfndef();
    void HandleEndif();
    void HandleElse();
    void SkipCurrentPreprocessorDirective();
    void SkipToNextBranch();

private:
    std::list<Token*> m_TokenList;
    std::list<Token*>::iterator m_Current;
    Tokenizer m_Tokenizer;
    MacroTable m_MacroTable;
    BranchStack m_BranchStack;
};


#endif // PREPROCESSOR_H_INCLUDED
