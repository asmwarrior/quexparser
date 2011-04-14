
#include "preprocessor.h"
#include "cchelper.h"
#include <iostream>
#include <fstream>
#include<iomanip>
using namespace std;

#include "expressionevaluator.h"



class FileLoader : public LoaderBase {
public:
    virtual const char* data() {
        return (const char*)buf;
    }
    virtual unsigned int length() {
        return buf_size;
    }
    virtual const char* fileName() {
        return name;
    }
public:
    FileLoader(const char* fileName) : name(fileName), buf(0), buf_size(0) {
        load();
    }
    bool isLoad() {
        return buf_size != 0;
    }
protected:
    void load() {
        ifstream in(name, ios::binary);
        if (!in.is_open())
            return;

        in.seekg(0,std::ios::end);
        buf_size = in.tellg();
        in.seekg(0);
        buf = new char[buf_size+2];

        buf[0] = 0;
        in.read(buf+1,buf_size);
        buf[buf_size+1] = '\0';

    }
    const char *    name;
    char*           buf;
    int             buf_size;
};

Preprocessor::Preprocessor()
{

}
Preprocessor::~Preprocessor() {

    std::list<RawToken*>::iterator it;
    //clear list
    for ( it=m_TokenList.begin() ; it != m_TokenList.end(); it++ ) {
        std::cout<<*(*it)<<std::endl;
        delete (*it);
    }

}

void  Preprocessor::LoadFile(cc_string filename) {
    FileLoader * loader = new FileLoader(filename.c_str());
    if (!loader->isLoad()) {
        delete loader;
        return ;
    }
    m_Tokenizer.Init(filename, loader);

}
void  Preprocessor::RunTest()
{
    RawToken*  pToken;

    try
    {
        while(true)
    {
        pToken  = new RawToken;
        if(m_Tokenizer.FetchToken(pToken))
        {
            //cout<<pToken->get_string()<<endl;

            if( TKN_PP_DEFINE <= pToken->type_id() && pToken->type_id() <= TKN_PP_ERROR)

            {
                switch (pToken->type_id())
                {

                case TKN_PP_DEFINE:
                {
                    delete pToken;
                    AddMacroDefinition();
                    break;
                }
                case TKN_PP_IF:
                {
                    delete pToken;
                    HandleIf();
                    break;
                }
                case TKN_PP_ELIF:
                {
                    delete pToken;
                    HandleElif();
                    break;
                }
                case TKN_PP_IFDEF :
                {
                    delete pToken;
                    HandleIfdef();
                    break;
                }
                case TKN_PP_IFNDEF:
                {
                    delete pToken;
                    HandleIfndef();
                    break;
                }
                case TKN_PP_ENDIF:
                {
                    delete pToken;
                    HandleEndif();
                    break;
                }
                case TKN_PP_ELSE:
                {
                    delete pToken;
                    HandleElse();
                    break;
                }
                case TKN_PP_PRAGMA:
                {
                    delete pToken;
                    SkipCurrentPreprocessorDirective();
                    break;
                }
                case TKN_PP_ERROR:
                {
                    delete pToken;
                    SkipCurrentPreprocessorDirective();
                    break;
                }
                case TKN_PP_UNDEF:
                {
                    delete pToken;
                    SkipCurrentPreprocessorDirective();
                    break;
                }
                default:
                    delete pToken;
                    SkipCurrentPreprocessorDirective();

                }


            }
            else
            {
                m_TokenList.push_back(pToken);
            }

        }
        else//EOF
        {
            delete pToken;
            break;
        }//if(m_Tokenizer.FetchToken(pToken))
    }//while(true)

    }
    catch(ParserException& e)
    {
        cout<< "end of file" <<endl;
    }




    DumpMacroTable();
}

void Preprocessor::AddMacroDefinition()
{
    RawToken token;
    MacroDefine entry;
    m_Tokenizer.FetchToken(&(entry.m_Name));
    m_Tokenizer.FetchToken(&token);

    if(token.type_id()==TKN_L_PAREN)//function like macro
    {
        entry.m_IsFunctionLike=true;
        //read the argument
        do{
            m_Tokenizer.FetchToken(&token);
            if(token.type_id()==TKN_R_PAREN)
                break;
            else if(token.type_id()!=TKN_COMMA)
                entry.m_Arguments.push_back(token);
        }
        while(true);
    }
    else
    {
        if(token.type_id()!=TKN_PP_FINISH)
            entry.m_DefineValue.push_back(token);
    }

    //read macro definition string
    do{
        m_Tokenizer.FetchToken(&token);
        if(token.type_id()!=TKN_PP_FINISH)
            entry.m_DefineValue.push_back(token);
        else
            break;
    }
    while(true);
    // add this entry
    m_MacroTable[entry.m_Name.get_text()]=entry;

}
void Preprocessor::DumpMacroTable()
{
  MacroTable::iterator it;
  // show content:
  for ( it=m_MacroTable.begin() ; it != m_MacroTable.end(); it++ )
  {
      (*it).second.Dump();
  }



}

RawToken*  Preprocessor::GetToken() {

    if( m_Current != m_TokenList.end()) {
        m_Current++;
        return *m_Current;
    } else {
        RawToken * p = new RawToken;
        m_Tokenizer.FetchToken(p);
        m_TokenList.push_back(p);
        return p;
    }

}
RawToken*  Preprocessor::CurrentToken() {
    return *m_Current;
}
RawToken*  Preprocessor::PeekToken(int step) {

//    for(int i=0;i<step;i++)
//    {
//
//    }
//
//    int n = (m_Index+step)%TOKEN_BUFFER_SIZE;;
//    while(m_PeekAvailable<step)
//    {
//        m_PeekAvailable++;
//        n = (m_Index+m_PeekAvailable)%TOKEN_BUFFER_SIZE;
//        DoAdvanceGetToken(n);
//    }
//    return &m_TokenBuffer[n];

    return NULL;

}
void  Preprocessor::RemoveBefore() {

}
bool  Preprocessor::MacroReplace(std::list<RawToken*> & macroDefine) {

}
bool  Preprocessor::ConstExpressionValue()
{
//    RawToken token;
//    int value;
//    while(true)
//    {
//        m_Tokenizer.FetchToken(&token);
//        if(token.type_id()==TKN_NUMBER)
//        {
//            value = atoi(token.get_text().c_str());
//            break;
//        }
//    }
//    SkipCurrentPreprocessorDirective();
//
//    if(value==0)
//        return false;
//    else
//        return true;

    RawToken token;
    vector<RawToken> exp;
    while(true)
    {
        m_Tokenizer.FetchToken(&token);
        exp.push_back(token);
        if(token.type_id()==TKN_PP_FINISH||token.type_id()==TKN_TERMINATION)
        {
            break;
        }
    }
    ConstExpression eval(this);
    return eval.expression_eval(&exp[0]);


}

void Preprocessor::HandleIf()
{
    //evaluate the current line
    // if true, level++ , return
    // if false, skip to #endif(level--) or #else or #elseif , they must be in the same level
    //, return

    BranchEntry entry;
    if(ConstExpressionValue()==true)
    {

        entry.m_Value=true;
        m_BranchStack.push(entry);
        return;
    }
    else //false
    {
        entry.m_Value=false;
        m_BranchStack.push(entry);
        SkipToNextBranch();
        return;
    }

}
void Preprocessor::HandleElif()
{
    // evaluate the current line
    // if true, return
    // if false, skip

    if(ConstExpressionValue()==true)
    {
        return;
    }
    else //false
    {
        SkipToNextBranch();
        return;
    }
}
void Preprocessor::HandleIfdef()
{
    //evaluate the current line
    // if ture, level++, return
    //

    RawToken token;
    m_Tokenizer.FetchToken(&token);
    bool InParentheses = false;

    if (token.type_id()==TKN_L_PAREN)
    {
        InParentheses = true;
        m_Tokenizer.FetchToken(&token);
    }
    bool exist=false;
    if(token.type_id()==TKN_IDENTIFIER)
    {
        //Chech this Id in the symbol table
        exist = CheckMacroExist(token.get_text());
    }

    if(InParentheses==true)//skip the right parenthesis
        m_Tokenizer.FetchToken(&token);


    BranchEntry entry;
    if(exist==true)
    {
        entry.m_Value=true;
        m_BranchStack.push(entry);
        return;
    }
    else //false
    {
        entry.m_Value=false;
        m_BranchStack.push(entry);
        SkipToNextBranch();
        return;
    }

}
void Preprocessor::HandleIfndef()
{
    RawToken token;
    m_Tokenizer.FetchToken(&token);
    bool InParentheses = false;

    if (token.type_id()==TKN_L_PAREN)
    {
        InParentheses = true;
        m_Tokenizer.FetchToken(&token);
    }
    bool exist=false;
    if(token.type_id()==TKN_IDENTIFIER)
    {
        //Chech this Id in the symbol table
        exist = CheckMacroExist(token.get_text());
    }

    if(InParentheses==true)//skip the right parenthesis
        m_Tokenizer.FetchToken(&token);


    BranchEntry entry;
    if(exist==false) // chech false!!!
    {
        entry.m_Value=true;
        m_BranchStack.push(entry);
        return;
    }
    else
    {
        entry.m_Value=true;
        m_BranchStack.push(entry);
        SkipToNextBranch();
        return;
    }

}
void Preprocessor::HandleEndif()
{
    //level go up, level--
    if(m_BranchStack.size()>0)
        m_BranchStack.pop();
    return;
}
void Preprocessor::HandleElse()
{
//do nothing, this branch must be used
//    if(last branch is true), we should skip this branch
//    else we should use this branch
    if(m_BranchStack.size()>0)
    {
        if(m_BranchStack.top().m_Value==true)
            SkipToNextBranch();
        else
            return; // this should be true
    }

}
void Preprocessor::SkipCurrentPreprocessorDirective()
{
    RawToken token;
    while(true)
    {
        m_Tokenizer.FetchToken(&token);
        if(token.type_id()==TKN_PP_FINISH||token.type_id()==TKN_TERMINATION)
        {
            break;
        }
    }

}
void Preprocessor::SkipToNextBranch()
{
    //In the same #if level
    int level = 0;
    RawToken token;
    while(true)
    {
        m_Tokenizer.FetchToken(&token);
        if(token.type_id()==TKN_PP_IF
           ||token.type_id()==TKN_PP_IFDEF
           ||token.type_id()==TKN_PP_IFNDEF)
        {
          level++;
        }
        else if(  token.type_id()==TKN_PP_ELIF
                ||token.type_id()==TKN_PP_ELSE)
        {
            if (level==0)//find the same level branch
            {
                if(token.type_id()==TKN_PP_ELIF)
                    HandleElif();
                else
                    HandleElse();
                return;
            }
        }
        else if (token.type_id()==TKN_PP_ENDIF)
        {

            if (level==0)//find the same level endif
            {
                HandleEndif();
                return;
            }
            else
                level--;
        }
    }

}

bool  Preprocessor::CheckMacroExist(std::string key)
{
    if(m_MacroTable.find(key)==m_MacroTable.end())
        return false;
    else
        return true;

}
