
#include "preprocessor.h"
#include "cchelper.h"
#include <iostream>
#include <fstream>
#include<iomanip>
using namespace std;

void DumpExp(vector<Token> & exp);

#include "expressionevaluator.h"
#include "timer.h"

#define PREPROCESSOR_DEBUG 0

#if PREPROCESSOR_DEBUG
#define TRACE(format, args...)\
    printf(format , ## args);\
    printf("\n")
#else
#define TRACE(format, ...)
#define DebugLog(format,...)
#endif



class FileLoader : public LoaderBase
{
public:
    virtual const char* data()
    {
        return (const char*)buf;
    }
    virtual unsigned int length()
    {
        return buf_size;
    }
    virtual const char* fileName()
    {
        return name;
    }
public:
    FileLoader(const char* fileName) : name(fileName), buf(0), buf_size(0)
    {
        load();
    }
    bool isLoad()
    {
        return buf_size != 0;
    }
protected:
    void load()
    {
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
Preprocessor::~Preprocessor()
{
    std::list<Token*>::iterator it;

    //clear list
    for ( it=m_TokenList.begin() ; it != m_TokenList.end(); it++ )
    {
        delete (*it);
    }

}
void Preprocessor::DumpTokenList()
{
    std::list<Token*>::iterator it;

    for ( it=m_TokenList.begin() ; it != m_TokenList.end(); it++ )
    {
        std::cout<<(*it)->line_number()<<":"<<(*it)->column_number()<<" "<<*(*it)<<std::endl;
    }
}

void  Preprocessor::LoadFile(cc_string filename)
{
    FileLoader * loader = new FileLoader(filename.c_str());
    if (!loader->isLoad())
    {
        delete loader;
        return ;
    }
    m_Tokenizer.Init(filename, loader);

}
void  Preprocessor::Preprocess()
{
    Token*  pToken;

    while(true)
    {
        pToken  = new Token;
        m_Tokenizer.FetchToken(pToken);

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
        else if( pToken->type_id() == TKN_IDENTIFIER )
        {
            if (CheckMacroExist(pToken->get_text()))
            {
                //We should do a macro replacement
                Token tok = *pToken;
                delete pToken;
                //macro exist
                MacroDefine &def = m_MacroTable[tok.get_text()];
                if(def.m_IsFunctionLike == true) //function like macro
                {

                    //read argument
                    int argNum = def.m_Arguments.size();
                    vector< vector<Token> > arg;
                    ReadMacroActualArgument(arg,argNum);

                    //replace
                    vector<Token> expand;
                    MacroExpansion(expand,def,arg);

                    //finally, insert the expanded tokens
                    for(vector<Token>::iterator itExpend=expand.begin() ; itExpend < expand.end(); itExpend++)
                    {
                        Token * p = new Token;
                        *p = *itExpend;
                        m_TokenList.push_back(p);
                    }
                }
                else                             //object like macro
                {
                    for(vector<Token>::iterator itDef=def.m_DefineValue.begin() ; itDef < def.m_DefineValue.end(); itDef++)
                    {
                        Token * p = new Token;
                        *p = *itDef;
                        m_TokenList.push_back(p);
                    }

                }

            } // not a macro, so just push it to normal token list
            else
            {
                m_TokenList.push_back(pToken);
            }
        }
        else if( pToken->type_id() == TKN_TERMINATION)
        {
            m_TokenList.push_back(pToken);
            break;
        }
        else
        {
            m_TokenList.push_back(pToken);
        }
    }
    m_Current = m_TokenList.begin();
}



void Preprocessor::AddMacroDefinition()
{
    Token token;
    MacroDefine entry;
    m_Tokenizer.FetchToken(&(entry.m_Name));
    m_Tokenizer.FetchToken(&token);

    if(token.type_id()==TKN_TERMINATION)
        return;

    if(token.type_id()==TKN_L_PAREN)   //function like macro #define A(a,b)  a+b
    {
        entry.m_IsFunctionLike=true;
        //read the formal parameters
        do
        {
            m_Tokenizer.FetchToken(&token);
            if(token.type_id()==TKN_TERMINATION)
                return;

            if(token.type_id()==TKN_R_PAREN)
                break;
            else if(token.type_id()!=TKN_COMMA)
                entry.m_Arguments.push_back(token);
        }
        while(true);
    }
    else                                //object like macro
    {
        if(token.type_id()!=TKN_PP_FINISH)
            entry.m_DefineValue.push_back(token);
    }

    //read macro definition string
    do
    {
        m_Tokenizer.FetchToken(&token);
        if(token.type_id()==TKN_TERMINATION)
            return;
        if(token.type_id()!=TKN_PP_FINISH)
            entry.m_DefineValue.push_back(token);
        else
            break;
    }
    while(true);
    // add this macro definition to the macro table
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

Token*  Preprocessor::GetToken()
{
    if( m_Current != m_TokenList.end())
    {
        Token * p = *m_Current;
        m_Current++;
        return p;
    }
    else
    {
        throw ParserException();
        return 0;
    }
}

Token*  Preprocessor::PeekToken(int step)
{
    std::list<Token*>::iterator peek = m_Current;
    for(int i=0;i<step-1;i++)
    {
        peek++;
        if(peek==m_TokenList.end())
            return 0;
    }
    return *(peek);
}
void Preprocessor::UngetToken()
{
    // do nothing!
    m_Current--;
}


bool  Preprocessor::ConstExpressionValue()
{
    //read the current logic line of the preprocessor directive
    Token token;
    vector<Token> exp;
    while(true)
    {
        m_Tokenizer.FetchToken(&token);
        if(token.type_id()==TKN_TERMINATION)
            return false;
        exp.push_back(token);
        if(token.type_id()==TKN_PP_FINISH)
        {
            break;
        }
    };

    if(ConstExpressionExpansion(exp)==false)
        return false;
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
    else     //false
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
    else     //false
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

    Token token;
    m_Tokenizer.FetchToken(&token);
    if(token.type_id()==TKN_TERMINATION)
        return;
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
    else     //false
    {
        entry.m_Value=false;
        m_BranchStack.push(entry);
        SkipToNextBranch();
        return;
    }

}
void Preprocessor::HandleIfndef()
{
    Token token;
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
    //remove the trailing pp_finish
    m_Tokenizer.FetchToken(&token);
    if(token.type_id()==TKN_TERMINATION)
        return;

    BranchEntry entry;
    if(exist==false)   // chech false!!!
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
    //remove pp_finish;
    Token tok;
    m_Tokenizer.FetchToken(&tok);
    if(tok.type_id()==TKN_TERMINATION)
        return;
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
    Token token;
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
    Token token;
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
            if (level==0)   //find the same level branch
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

            if (level==0)   //find the same level endif
            {
                HandleEndif();
                return;
            }
            else
                level--;
        }
        else if(token.type_id()==TKN_TERMINATION)
            return;
    }

}

bool  Preprocessor::CheckMacroExist(std::string key)
{
    if(m_MacroTable.find(key)==m_MacroTable.end())
        return false;
    else
        return true;

}
void Preprocessor::RunTest()
{
    Preprocess();
    DumpMacroTable();
    DumpTokenList();
}
void  Preprocessor::RunTestPerformance()
{
    Token*  pToken = new Token;

    CTimer a;
    a.reset();

    while(true)
    {
        m_Tokenizer.FetchToken(pToken);

        //cout<<pToken->get_string()<<endl;
        //m_TokenList.push_back(pToken);

        if(pToken->type_id()==TKN_TERMINATION)
            break;

    }
    std::cout<<a.GetCurrentTime()<<std::endl;
}

void DumpExp(vector<Token> & exp)
{
    cout<<"Dump Start"<<endl;
    for(vector<Token>::iterator it=exp.begin() ; it < exp.end(); it++)
    {
        cout<<*it<<endl;
    }
    cout<<"Dump End"<<endl;

}

bool Preprocessor::ConstExpressionExpansion(vector<Token> & exp)
{
    // defined (XXX) will correctly calculated first

    bool needNextPass = false;
    int count = 0;

    //simply we need a count variable to avoid some recursion
    //e.g.
    //  #define A B
    //  #define B A
    //  These kind of recursion can be avoid.

    while(count<2) //for simplicity, we only do two round macro expansion, this can avoid recursion
    {
        count++;
        needNextPass = false;

        for(vector<Token>::iterator it=exp.begin() ; it < exp.end(); it++)
        {
            //Convert defined (ID) or defined ID to a Number Token
            if((*it).type_id() == TKN_DEFINED )
            {
                vector<Token>::iterator beginOfDefineCheck = it;
                bool value = false;
                it++;
                bool InParentheses = false;
                if ((*it).type_id()==TKN_L_PAREN)
                {
                    InParentheses = true;
                    it++;
                }

                if((*it).type_id()==TKN_IDENTIFIER)
                {
                    //Check this Id in the symbol table
                    bool exist = CheckMacroExist((*it).get_text());
                    if (exist==true)
                        value = true;
                }
                else
                    value = false;

                if(InParentheses==true)//skip the right parenthesis
                    it++;

                // remove from beginOfDefineCheck to endIdx by replace it with a TKN_NUMBER token
                (*beginOfDefineCheck).set(TKN_NUMBER); //Set the token ID

                cc_string valueString;
                if(value==true) //Set the number string
                    valueString="1";
                else
                    valueString="0";
                (*beginOfDefineCheck).set_text(valueString);

                //remove the left tokens: either (ID) or ID
                exp.erase(beginOfDefineCheck+1,it+1);

                //adjust the iterator
                it=beginOfDefineCheck;
            }
            else if((*it).type_id() == TKN_IDENTIFIER ) //Try to expand every identifier
            {
                if (CheckMacroExist((*it).get_text()))
                {
                    //macro exist
                    MacroDefine &def = m_MacroTable[(*it).get_text()];
                    if(def.m_IsFunctionLike == true)
                    {

                        vector<Token>::iterator IDIterator = it;
                        //expand the arguments

                        //read argument from macro definition
                        int argNum = def.m_Arguments.size();
                        vector< vector<Token> > arg;
                        vector<Token> single;

                        //consider semicolon and top level parentheses
                        it++; // move the iterator to left parenthesis

                        for(int num = 0; num<argNum; num++)
                        {
                            it++; // go to the next id
                            arg.push_back(single);
                            while(   (*it).type_id()!=TKN_COMMA
                                     && (*it).type_id()!=TKN_R_PAREN )
                            {

                                arg[num].push_back(*it);
                                it++;
                            }
                            #if PREPROCESSOR_DEBUG
                            DumpExp(arg[num]);
                            #endif
                        }

                        // *it should be a right parenthesis
                        vector<Token>::iterator rightParenIterator = it;

                        //generated new tokens
                        vector<Token> expand;
                        MacroExpansion(expand,def,arg);

                        //finally, insert the expand (replacement)
                        //remove old tokens
                        it=exp.erase(IDIterator,rightParenIterator+1); //remove the current tokens;

                        // insert new tokens
                        for(vector<Token>::iterator itExpand=expand.begin();
                            itExpand < expand.end();
                            itExpand++)
                        {
                            it=exp.insert(it,*itExpand);
                        }

                        #if PREPROCESSOR_DEBUG
                        DumpExp(exp);
                        #endif
                    }// function like macro handling
                    else
                    {
                        exp.erase(it); //remove the current token;
                        //replacement
                        for(vector<Token>::iterator itDef=def.m_DefineValue.begin() ; itDef < def.m_DefineValue.end(); itDef++)
                        {
                            it=exp.insert(it,*itDef);
                        }
                        #if PREPROCESSOR_DEBUG
                        DumpExp(exp);
                        #endif
                    }
                    //As we do a replacement, so we need a next pass
                    needNextPass = true;
                }
                else //This identifier is not a macro, so we have no way to calculate the expression
                {
                    cout<<"Can not expand an identifier:"<<(*it).type_id()<<endl;
                    cout<<"return false"<<endl;
                    return false; //this identifier is not defined, so return false;
                }
            }
        }
        if (needNextPass == false)
            break;
    }

    //dump the result
    cout<<"After Expansion\n";
    for(vector<Token>::iterator it=exp.begin() ; it < exp.end(); it++)
        cout<<*it<<endl;
    return true;

}
void Preprocessor::MacroExpansion(vector<Token> &expand, MacroDefine &def, vector<vector<Token> > &arg)
{
    //loop the definition, and do the expansion
    for(vector<Token>::iterator itDef=def.m_DefineValue.begin();
        itDef < def.m_DefineValue.end();
        itDef++)
    {
        if((*itDef).type_id() == TKN_IDENTIFIER )
        {
            bool replaced = false;
            //check it is whether a parameter
            for(size_t i = 0; i<def.m_Arguments.size(); i++)
            {
                if((*itDef).get_text()==def.m_Arguments[i].get_text())
                {
                    //do a replacement
                    replaced = true;
                    expand.insert(expand.end(),arg[i].begin(),arg[i].end());
                    #if PREPROCESSOR_DEBUG
                    DumpExp(expand);
                    #endif
                }
            }
            if(replaced==false)
                expand.push_back(*itDef);
        }
        else
            expand.push_back(*itDef);

        #if PREPROCESSOR_DEBUG
        DumpExp(expand);
        #endif
    }
}

void Preprocessor::ReadMacroActualArgument(vector<vector<Token> > &arg, int argNum)
{

    vector<Token> single;
    //consider semicolon and top-level parentheses
    Token tok;
    m_Tokenizer.FetchToken(&tok); // should be a left parenthesis
    //read next
    if(tok.type_id()==TKN_TERMINATION)
        return;

    for(int num = 0; num<argNum; num++)
    {
        arg.push_back(single);
        do
        {
            m_Tokenizer.FetchToken(&tok);
            if(tok.type_id()==TKN_TERMINATION)
                return;
            if(tok.type_id()!=TKN_COMMA
                    && tok.type_id()!=TKN_R_PAREN )
                arg[num].push_back(tok);
            else
                break;

        }// go to the next id
        while( true );

        #if PREPROCESSOR_DEBUG
        DumpExp(arg[num]);
        #endif
    }
}
