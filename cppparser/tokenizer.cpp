/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include "tokenizer.h"

#define TOKENIZER_DEBUG_OUTPUT 1

#if TOKENIZER_DEBUG_OUTPUT
    #include <stdio.h>
    #define TRACE(format, args...)\
    printf( format , ## args)
#else
    #define TRACE(format, ...)
#endif



#define ELEMENT_TYPE uint8_t

#include <iostream>

using namespace std;

// static
std::map<cc_string,cc_string> Tokenizer::s_Replacements;
QUEX_TYPE_CHARACTER Tokenizer::s_QuexBuffer[4] = {0,0,0,0};

Tokenizer::Tokenizer(const cc_string& filename)
    : m_Filename(filename),
    m_BufferLen(0),
    m_PeekAvailable(0),
    m_IsOK(false),
    m_Quex((QUEX_TYPE_CHARACTER*)s_QuexBuffer,4,(QUEX_TYPE_CHARACTER*)s_QuexBuffer+1),
    m_IsEOF(false),
    m_Index(0)

{

    cout<<"##----------QUEX Information---------------\n";
    cout<< m_Quex.version()<<endl;
    cout<<"##-----------------------------------------\n";
    if (!m_Filename.empty())
        Init(m_Filename);
}

Tokenizer::~Tokenizer()
{

}

bool Tokenizer::Init(const cc_string& filename, LoaderBase* loader)
{
    m_pLoader = loader;
    BaseInit();
    if (filename.empty())
    {
        if (m_Filename.empty())
        {
            TRACE(cc_text("Init() : Called without filename."));
            return false;
        }
    }
    else
    {
        m_Filename = filename;
        TRACE(cc_text("Init() : m_Filename='%s'"), m_Filename.c_str());
    }

    if (!wxFileExists(m_Filename))
    {
        TRACE(cc_text("Init() : File '%s' does not exist."), m_Filename.c_str());
        return false;
    }

    if (!ReadFile())
    {
        TRACE(cc_text("Init() : File '%s' could not be read."), m_Filename.c_str());
        return false;
    }

    if (!m_BufferLen)
    {
        TRACE(cc_text("Init() : File '%s' is empty."), m_Filename.c_str());
        return false;
    }

    m_IsOK = true;
    return true;
}

bool Tokenizer::InitFromBuffer(const cc_string& buffer)
{
    BaseInit();
    m_BufferLen = buffer.length();
    m_Buffer.resize(m_BufferLen + 1); // + 1 => sentinel
    m_Buffer = buffer;
    m_Buffer += cc_text('\0');
    m_IsOK = true;
    m_Filename.clear();
    const char * pBuffer = m_Buffer.data();
    m_Quex.reset_buffer((QUEX_TYPE_CHARACTER*)pBuffer,
                            m_BufferLen+2,
                            (QUEX_TYPE_CHARACTER*)pBuffer+m_BufferLen+1);
    return true;
}

void Tokenizer::BaseInit()
{
    m_BufferLen           = 0;
    m_IsOK                = false;
    m_Buffer.clear();
}

bool Tokenizer::ReadFile()
{
    bool success = false;
    cc_string fileName = cc_text("");
    if (m_pLoader)
    {
        fileName = m_pLoader->fileName();

        const char * pBuffer = m_pLoader->data();
        m_BufferLen = m_pLoader->length();

        if( m_BufferLen != 0)
            success = true;


        m_Quex.reset_buffer((QUEX_TYPE_CHARACTER*)pBuffer,
                                       m_BufferLen+2,
                                       (QUEX_TYPE_CHARACTER*)pBuffer+m_BufferLen+1);

        (void)m_Quex.token_p_switch(&m_TokenBuffer[0]);

        cout<< "Macro: QUEX_SETTING_BUFFER_SIZE=" << (int)QUEX_SETTING_BUFFER_SIZE <<endl;

        return true;

    }
    else
    {
        if (!wxFileExists(m_Filename))
            return false;

        // open file
        wxFile file(m_Filename);

        if (!cbRead(file, m_Buffer))
            return false;

        fileName = m_Filename;
        success = true;
    }

    return success;
}


Token* Tokenizer::GetToken()
{
    //m_UndoToken = m_Token;
        // move a forward step
        m_Index++;
        m_Index%=TOKEN_BUFFER_SIZE;

    if(m_PeekAvailable>0)
    {
        m_PeekAvailable--;
        assert(m_PeekAvailable>=0);
    }
    else

        DoAdvanceGetToken(m_Index);


    return &m_TokenBuffer[m_Index];
}

Token* Tokenizer::CurrentToken()
{
    return &m_TokenBuffer[m_Index];
}


void  Tokenizer::DoAdvanceGetToken(int n)
{
    DoGetToken(n);
}


Token* Tokenizer::PeekToken(int step)
{
    assert(step<=2);
    int n = (m_Index+step)%TOKEN_BUFFER_SIZE;;
    while(m_PeekAvailable<step)
    {
        m_PeekAvailable++;
        n = (m_Index+m_PeekAvailable)%TOKEN_BUFFER_SIZE;
        DoAdvanceGetToken(n);
    }
    return &m_TokenBuffer[n];
}

void Tokenizer::UngetToken()
{

}

void Tokenizer::DoGetToken(int n)
{

    //fill the m_TokenBuffer[n];
    m_TokenBuffer[n].text.clear();
    (void)m_Quex.token_p_switch(&m_TokenBuffer[n]);


    QUEX_TYPE_TOKEN_ID id = m_Quex.receive();

    if( id == TKN_TERMINATION )
    {
        m_IsEOF = true;
        throw ParserException();
    }
    else if ( id == TKN_FAILURE )
    {
        cout<< "fail" << " line=" << m_TokenBuffer[n].line_number() << "colunm=" << m_TokenBuffer[n].column_number()<<endl;
    }
    else if( id == TKN_FAILURE_STRING )
    {
        cout<< "fail string" << " line=" << m_TokenBuffer[n].line_number() << "colunm=" << m_TokenBuffer[n].column_number()<<endl;
    }

    //cout<< "DoGetToken(): " << "line=" << m_TokenBuffer[n].line_number() << "colunm=" << m_TokenBuffer[n].column_number();
    //cout<<m_TokenBuffer[n].get_text()<<endl;
}


cc_string Tokenizer::MacroReplace(const cc_string str)
{
    return str;
}


void Tokenizer::RunTest()
{
    Token* token;

    while(1)
    {
        token = GetToken();

        cout<<*token<< " line=" << token->line_number() << "colunm=" << token->column_number()<<endl;

        if( token->type_id()== TKN_TERMINATION )
        {
            break;
        }

    }

}

void Tokenizer::FetchToken(Token * pToken)
{
    pToken->text.clear();
    (void)m_Quex.token_p_switch(pToken);
    QUEX_TYPE_TOKEN_ID id = m_Quex.receive();

}

