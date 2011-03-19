#ifndef PREPROCESSOR_H_INCLUDED
#define PREPROCESSOR_H_INCLUDED

#include<list>
#include"tokenizer.h"

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
private:
    std::list<RawToken*> m_TokenList;
    std::list<RawToken*>::iterator m_Current;
    Tokenizer m_Tokenizer;

};


#endif // PREPROCESSOR_H_INCLUDED
