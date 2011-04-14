#ifndef PREPROCESSOR_H_INCLUDED
#define PREPROCESSOR_H_INCLUDED

#include<list>
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
private:
    std::list<RawToken*> m_TokenList;
    std::list<RawToken*>::iterator m_Current;
    Tokenizer m_Tokenizer;
    MacroTable m_MacroTable;

};


#endif // PREPROCESSOR_H_INCLUDED
