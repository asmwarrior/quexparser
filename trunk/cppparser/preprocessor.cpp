
#include "preprocessor.h"
#include "cchelper.h"
#include <iostream>
#include <fstream>
#include<iomanip>
using namespace std;

void DumpExp(vector<RawToken> & exp);

#include "expressionevaluator.h"

#include "timer.h"



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
    std::list<RawToken*>::iterator it;
    //clear list
    for ( it=m_TokenList.begin() ; it != m_TokenList.end(); it++ )
    {
        delete (*it);
    }

}
void Preprocessor::DumpTokenList()
{
    std::list<RawToken*>::iterator it;
    //clear list
    for ( it=m_TokenList.begin() ; it != m_TokenList.end(); it++ )
    {
        std::cout<<(*it)->line_number()<<":"<<(*it)->column_number()<<" "<<*(*it)<<std::endl;
        //delete (*it);
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
    RawToken*  pToken;

    while(true)
    {
        pToken  = new RawToken;
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
                //We should do a macro replacement, so
                RawToken tok = *pToken;
                delete pToken;
                //macro exist
                MacroDefine &def = m_MacroTable[tok.get_text()];
                if(def.m_IsFunctionLike == true) //function like
                {

                    //read argument
                    int argNum = def.m_Arguments.size();
                    vector< vector<RawToken> > arg;
                    ReadMacroActualArgument(arg,argNum);

                    //replace
                    vector<RawToken> expend;
                    MacroExpansion(expend,def,arg);

                    //finally, insert the expended
                    for(vector<RawToken>::iterator itExpend=expend.begin() ; itExpend < expend.end(); itExpend++)
                    {
                        RawToken * p = new RawToken;
                        *p = *itExpend;
                        m_TokenList.push_back(p);
                    }
                }
                else                             //object like
                {
                    for(vector<RawToken>::iterator itDef=def.m_DefineValue.begin() ; itDef < def.m_DefineValue.end(); itDef++)
                    {
                        RawToken * p = new RawToken;
                        *p = *itDef;
                        m_TokenList.push_back(p);
                    }

                }


            } // not a macro definition, so just push it
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
    RawToken token;
    MacroDefine entry;
    m_Tokenizer.FetchToken(&(entry.m_Name));
    m_Tokenizer.FetchToken(&token);
    if(token.type_id()==TKN_TERMINATION)
        return;

    if(token.type_id()==TKN_L_PAREN)   //function like macro
    {
        entry.m_IsFunctionLike=true;
        //read the argument
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
    else
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

RawToken*  Preprocessor::GetToken()
{
    if( m_Current != m_TokenList.end())
    {
        RawToken * p = *m_Current;
        m_Current++;
        return p;
    }
    else
    {
        throw ParserException();
        return 0;
    }
}

RawToken*  Preprocessor::PeekToken(int step)
{
    std::list<RawToken*>::iterator peek = m_Current;
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

    RawToken token;
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
    RawToken tok;
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
    RawToken*  pToken = new RawToken;

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

void DumpExp(vector<RawToken> & exp)
{
    cout<<"Dump Start"<<endl;
    for(vector<RawToken>::iterator it=exp.begin() ; it < exp.end(); it++)
    {
        cout<<*it<<endl;
    }
    cout<<"Dump End"<<endl;

}

bool Preprocessor::ConstExpressionExpansion(vector<RawToken> & exp)
{
    // defined (XXX) will correctly caculated first

    bool needNextPass = false;
    int count = 0;

    //simply we need a count variable to avoid some recursion
    //e.g.
    //  #define A B
    //  #define B A
    //  These kind of recursion can be avoid.

    while(count<2)
    {
        count++;
        needNextPass = false;

        for(vector<RawToken>::iterator it=exp.begin() ; it < exp.end(); it++)
        {
            //Convert defined (ID) or defined ID to a Number Token
            if((*it).type_id() == TKN_DEFINED )
            {
                vector<RawToken>::iterator beginOfDefineCheck = it;
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
                    //Chech this Id in the symbol table
                    bool exist = CheckMacroExist((*it).get_text());
                    if (exist==true)
                        value = true;
                }
                else
                    value = false;

                if(InParentheses==true)//skip the right parenthesis
                    it++;

                // remove from beginOfDefineCheck to endIdx
                (*beginOfDefineCheck).set(TKN_NUMBER);
                cc_string valueString;
                if(value==true)
                    valueString="1";
                else
                    valueString="0";

                (*beginOfDefineCheck).set_text(valueString);
                exp.erase(beginOfDefineCheck+1,it+1);
                it=beginOfDefineCheck;
            }
            else if((*it).type_id() == TKN_IDENTIFIER )
            {
                if (CheckMacroExist((*it).get_text()))
                {
                    //macro exist
                    MacroDefine &def = m_MacroTable[(*it).get_text()];
                    if(def.m_IsFunctionLike == true)
                    {

                        vector<RawToken>::iterator IDIterator = it;
                        //expend the augument
                        //read argument
                        int argNum = def.m_Arguments.size();
                        vector< vector<RawToken> > arg;
                        vector<RawToken> single;
                        //consider semicolon and toplevel parenthese
                        it++; // move to left parenthesis

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
                            DumpExp(arg[num]);
                            //it++;
                        }

                        //*it should be a right parenthesis
                        vector<RawToken>::iterator rightParenIterator = it;
                        //replace
                        vector<RawToken> expand;

                        MacroExpansion(expand,def,arg);

                        //finally, insert the expend
                        it=exp.erase(IDIterator,rightParenIterator+1); //remove the current tok;
                        //general replacement,quite simple
                        for(vector<RawToken>::iterator itExpand=expand.begin() ; itExpand < expand.end(); itExpand++)
                        {
                            it=exp.insert(it,*itExpand);
                        }
                        DumpExp(exp);

                    }
                    else
                    {
                        exp.erase(it); //remove the current tok;
                        //general replacement,quite simple
                        for(vector<RawToken>::iterator itDef=def.m_DefineValue.begin() ; itDef < def.m_DefineValue.end(); itDef++)
                        {
                            it=exp.insert(it,*itDef);
                        }
                        DumpExp(exp);

                    }

                    //As we do a replacement, so we need a next pass
                    needNextPass = true;
                }
                else
                {
                    cout<<"Can not expend an identifier:"<<(*it).type_id()<<endl;
                    cout<<"return false"<<endl;
                    return false; //this identifier is not defined, so return false;
                }
            }
        }
        if (needNextPass == false)
            break;

    }

    //dump the result
    cout<<"After Expension\n";
    for(vector<RawToken>::iterator it=exp.begin() ; it < exp.end(); it++)
        cout<<*it<<endl;
    return true;

}
void Preprocessor::MacroExpansion(vector<RawToken> &expend,MacroDefine &def, vector<vector<RawToken> > &arg)
{
    //loop the definition, and do the expension
    for(vector<RawToken>::iterator itDef=def.m_DefineValue.begin() ; itDef < def.m_DefineValue.end(); itDef++)
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
                    expend.insert(expend.end(),arg[i].begin(),arg[i].end());

                    DumpExp(expend);

                }
            }
            if(replaced==false)
                expend.push_back(*itDef);

        }
        else
            expend.push_back(*itDef);
        DumpExp(expend);
    }
}

void Preprocessor::ReadMacroActualArgument(vector<vector<RawToken> > &arg, int argNum)
{

    vector<RawToken> single;
    //consider semicolon and toplevel parenthese
    RawToken tok;
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
        DumpExp(arg[num]);
    }

}
