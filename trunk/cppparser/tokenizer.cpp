/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 * $Revision: 5945 $
 * $Id: tokenizer.cpp 5945 2009-12-01 13:06:54Z mortenmacfly $
 * $HeadURL: svn://svn.berlios.de/codeblocks/trunk/src/plugins/codecompletion/parser/tokenizer.cpp $
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

// static
std::map<cc_string,cc_string> Tokenizer::s_Replacements;

Tokenizer::Tokenizer(const cc_string& filename)
    : m_Filename(filename),
    m_BufferLen(0),
    m_PeekAvailable(false),
    m_IsOK(false),
    m_pQuex(0)

{

    if (!m_Filename.empty())
        Init(m_Filename);
}

Tokenizer::~Tokenizer()
{
    if(m_pQuex)
        delete m_pQuex;
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
    m_Buffer += cc_text(' ');
    m_IsOK = true;
    m_Filename.clear();
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

        if(m_pQuex)
            delete m_pQuex;

        m_pQuex = new quex::tiny_lexer((QUEX_TYPE_CHARACTER*)pBuffer,
                                       m_BufferLen,
                                       (QUEX_TYPE_CHARACTER*)pBuffer+m_BufferLen-1);

        (void)m_pQuex->token_p_switch(&m_QuexToken);

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


RawToken* Tokenizer::GetToken()
{
    m_UndoToken = m_Token;

    if(m_PeekAvailable)
    {
        m_Token  = m_PeekToken;
    }
    else

        DoAdvanceGetToken(m_Token);


    m_PeekAvailable = false;

    return &m_Token;
}


void  Tokenizer::DoAdvanceGetToken(RawToken & tk)
{
    while (true)
    {
        DoGetToken(tk);

        if( QUEX_TKN_PP_IF <= tk.id && tk.id <= QUEX_TKN_PP_ERROR)
        {
            //loop until we find a QUEX_TKN_QUEX_TKN_PP_FINISH
            do
            {
                DoGetToken(tk);
            }while(QUEX_TKN_PP_FINISH != tk.id);
        }
        else if (tk.id == QUEX_TKN_PP_DEFINE)
        {
            cc_string str;
            // read to the QUEX_TKN_PP_FINISH
            do
            {
                str << tk.text << " ";
                DoGetToken(tk);
            }while(QUEX_TKN_PP_FINISH != tk.id);
            str << tk.text;
            tk.text = str;
            break;
        }
        else
        {
            break;
        }
    }
}




RawToken* Tokenizer::PeekToken()
{
    if(!m_PeekAvailable)
    {
        m_PeekAvailable = true;
        DoAdvanceGetToken(m_PeekToken);
    }
    return &m_PeekToken;
}

void Tokenizer::UngetToken()
{
    m_Token      = m_UndoToken;
    m_PeekToken  = m_Token;
    m_PeekAvailable  = true;
}

void Tokenizer::DoGetToken(RawToken & tk)
{
    QUEX_TYPE_TOKEN_ID id = m_pQuex->receive();


    if( id == QUEX_TKN_TERMINATION )
    {
        m_IsEOF = true;
        tk.text = "";
    }
    else if (id == QUEX_TKN_PP_FINISH)
    {
        tk.text = "[EndOfPP]\n";
    }
    else
    {
        tk.text = (char*)(m_QuexToken.get_text().c_str());
    }
    tk.id     = id;
    tk.line   = m_QuexToken.line_number();
    tk.column = m_QuexToken.column_number();

    //cout<<m_QuexToken<< " line=" << m_QuexToken.line_number() << "colunm=" << m_QuexToken.column_number()<<endl;
}


cc_string Tokenizer::MacroReplace(const cc_string str)
{
    return str;
}


void Tokenizer::RunTest()
{
    QUEX_TYPE_TOKEN_ID id;

    while(1)
    {
        id = m_pQuex->receive();

        cout<<m_QuexToken<< " line=" << m_QuexToken.line_number() << "colunm=" << m_QuexToken.column_number()<<endl;

        if( id == QUEX_TKN_TERMINATION )
        {
            break;
        }

    }

}


