/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "cchelper.h"
#include "tiny_lexer"
using namespace std;

struct RawToken
{
    cc_string text;
    int line;
    int id;
};

class Tokenizer
{
public:
    Tokenizer(const cc_string& filename = cc_text(""));
    ~Tokenizer();

    bool Init(const cc_string& filename = cc_text(""), LoaderBase* loader = 0);
    bool InitFromBuffer(const cc_string& buffer);

    RawToken*  GetToken();
    RawToken*  PeekToken();
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
    void DoGetToken(RawToken & tk);
    cc_string FixArgument(cc_string src);
    bool ReadFile();

    void HandleConditionPreprocessor();
    void  DoAdvanceGetToken(RawToken & tk);


private:

    inline const cc_string& ThisOrReplacement(const cc_string& str) const
    {
        map<cc_string,cc_string>::const_iterator it = s_Replacements.find(str);
        if (it != s_Replacements.end())
            return it->second;
        return str;
    };

    cc_string MacroReplace(const cc_string str);

    cc_string         m_Filename;

    cc_string         m_Buffer;
    unsigned int     m_BufferLen;

    bool             m_PeekAvailable;
    RawToken         m_Token;
    RawToken         m_PeekToken;
    RawToken         m_UndoToken;

    bool  m_IsOK;
    bool m_IsEOF;

    LoaderBase*      m_pLoader;
    quex::tiny_lexer   m_Quex;
    quex::Token        m_QuexToken;


    void RemovePP(RawToken & tk);



public:

     void RunTest();

    //static functions
    static void SetReplacementString(const cc_string& from, const cc_string& to)
    {
        s_Replacements.insert(s_Replacements.end(), std::make_pair(from, to));
    };

    static void eraseReplacementString(const cc_string& from)
    {
        map<cc_string,cc_string>::iterator it = s_Replacements.find(from);
        if (it != s_Replacements.end())
            s_Replacements.erase(it);
    };

    static map<cc_string,cc_string>& GetTokenReplacementsMap()
    {
        return s_Replacements;
    };

    static map<cc_string,cc_string> s_Replacements;

};

#endif // TOKENIZER_H


