/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "cchelper.h"
#include "tiny_lexer"

#include <map>


typedef quex::Token RawToken;


class ParserException
{
    int type;
    cc_string text;
};

class Tokenizer
{
public:
    Tokenizer(const cc_string& filename = cc_text(""));
    ~Tokenizer();

    bool Init(const cc_string& filename = cc_text(""), LoaderBase* loader = 0);
    bool InitFromBuffer(const cc_string& buffer);

    RawToken*  GetToken();
    RawToken*  CurrentToken();
    RawToken*  PeekToken(int step = 1);

    void UngetToken();


    const cc_string& GetFilename() const
    {
        return m_Filename;
    };

    bool IsOK() const
    {
        return m_IsOK;
    };

    bool IsEOF() const
    {
        return m_IsEOF;
    };

    bool NotEOF() const
    {
        return !m_IsEOF;
    };

protected:
    void BaseInit();
    void DoGetToken(int n);
    cc_string FixArgument(cc_string src);
    bool ReadFile();

private:

    inline const cc_string& ThisOrReplacement(const cc_string& str) const
    {
        std::map<cc_string,cc_string>::const_iterator it = s_Replacements.find(str);
        if (it != s_Replacements.end())
            return it->second;
        return str;
    };

    cc_string MacroReplace(const cc_string str);

    cc_string         m_Filename;

    cc_string         m_Buffer;
    unsigned int     m_BufferLen;

    int             m_PeekAvailable;

    bool  m_IsOK;
    bool  m_IsEOF;

    LoaderBase         *m_pLoader;
    quex::tiny_lexer   m_Quex;

    #define TOKEN_BUFFER_SIZE 4
    quex::Token      m_TokenBuffer[TOKEN_BUFFER_SIZE];
    int              m_Index;

    //initialized buffer
    static QUEX_TYPE_CHARACTER s_QuexBuffer[4];

    /** Internally handle and strip all the preprocessor directive*/
    void DoAdvanceGetToken(int n);

public:

     void RunTest();

    //static functions
    static void SetReplacementString(const cc_string& from, const cc_string& to)
    {
        s_Replacements.insert(s_Replacements.end(), std::make_pair(from, to));
    };

    static void eraseReplacementString(const cc_string& from)
    {
        std::map<cc_string,cc_string>::iterator it = s_Replacements.find(from);
        if (it != s_Replacements.end())
            s_Replacements.erase(it);
    };

    static std::map<cc_string,cc_string>& GetTokenReplacementsMap()
    {
        return s_Replacements;
    };

    static std::map<cc_string,cc_string> s_Replacements;

};

#endif // TOKENIZER_H


