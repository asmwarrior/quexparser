
#include "preprocessor.h"
#include "cchelper.h"
#include <iostream>
#include <fstream>
#include<iomanip>
using namespace std;



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

Preprocessor::Preprocessor() {

}
Preprocessor::~Preprocessor() {

    std::list<RawToken*>::iterator it;
    //clear list
    for ( it=m_TokenList.begin() ; it != m_TokenList.end(); it++ ) {
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
void  Preprocessor::RunTest() {
    RawToken*  pToken;

    while(true) {
        pToken  = new RawToken;
        if(m_Tokenizer.FetchToken(pToken)) {
            //cout<<pToken->get_string()<<endl;
            if(pToken->type_id()==TKN_PP_DEFINE)
            {
                delete pToken;
                AddMacroDefinition();
            }
            else
                m_TokenList.push_back(pToken);
        } else {
            delete pToken;
            break;
        }

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
    m_MacroTable[entry.m_Name]=entry;

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
bool  Preprocessor::ConstExpressionValue() {

}
