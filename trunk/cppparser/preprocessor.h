#ifndef PREPROCESSOR_H_INCLUDED
#define PREPROCESSOR_H_INCLUDED

#include<list>
#include<stack>
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
            for(int i=0;i<m_Arguments.size();i++)
                std::cout<<m_Arguments[i].get_string()<<std::endl;
        }
        std::cout<<"VALUE:"<<std::endl;
        for(int i=0;i<m_DefineValue.size();i++)
            std::cout<<m_DefineValue[i].get_string()<<std::endl;
    }

    RawToken m_Name;
    bool m_IsFunctionLike;
    std::vector<RawToken> m_Arguments;
    std::vector<RawToken> m_DefineValue;
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

    RawToken*  GetToken();
    RawToken*  CurrentToken();
    RawToken*  PeekToken(int step = 1);
    void  RemoveBefore();
    bool  MacroReplace(std::list<RawToken*> & macroDefine);
    bool  ConstExpressionValue();
    void  LoadFile(cc_string filename);
    void  RunTest();
    void  AddMacroDefinition();
    void  DumpMacroTable();

    void MacroExpension(std::vector<RawToken> & exp);

    void HandleIf();
    void HandleElif();
    void HandleIfdef();
    void HandleIfndef();
    void HandleEndif();
    void HandleElse();
    void SkipCurrentPreprocessorDirective();
    void SkipToNextBranch();

    bool CheckMacroExist(std::string key);
	void  RunTestPerformance();
private:
    std::list<RawToken*> m_TokenList;
    std::list<RawToken*>::iterator m_Current;
    Tokenizer m_Tokenizer;
    MacroTable m_MacroTable;
    BranchStack m_BranchStack;

};


#endif // PREPROCESSOR_H_INCLUDED
