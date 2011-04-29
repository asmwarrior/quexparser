/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include "parserthread.h"

#define PARSERTHREAD_DEBUG_OUTPUT 1

#include <stdio.h>

#if PARSERTHREAD_DEBUG_OUTPUT

#define TRACE(format, args...)\
    printf(format , ## args);\
    printf("\n")
#else
#define TRACE(format, ...)
#define DebugLog(format,...)
#endif


ParserThread::ParserThread(Parser* parent,
                           const cc_string& bufferOrFilename,
                           bool isLocal,
                           ParserThreadOptions& parserThreadOptions,
                           TokensTree* tokensTree) :
    m_Tokenizer(),
    m_pParent(parent),
    m_pTokensTree(tokensTree),
    m_Filename(cc_text("")),
    m_FileSize(0),
    m_FileIdx(0),
    m_IsLocal(isLocal),
    m_Options(parserThreadOptions),
    m_IsBuffer(parserThreadOptions.useBuffer),
    m_Buffer(bufferOrFilename)
{

}

ParserThread::~ParserThread()
{
    //dtor

    // wait for file loader object to complete (can't abort it)
    if (m_Options.loader)
    {
        m_Options.loader->Sync();
        if (m_Options.loader)
        {
            delete m_Options.loader;
            m_Options.loader = NULL;
        }
    }
}

void ParserThread::Log(const cc_string& log)
{
//    Manager::ProcessPendingEvents();
}

void ParserThread::SetTokens(TokensTree* tokensTree)
{
    m_pTokensTree = tokensTree;
}

void* ParserThread::DoRun()
{

    return 0L;
}


void ParserThread::SkipToOneOfId(const int * idArray, const int num)
{
    RawToken * tk;
    tk = m_Tokenizer.GetToken();
    bool find = false;
    int id = tk->type_id();

    while(true)
    {
        //check the id is in idArray
        for(int i = 0; i<num; i++)
        {
            if(id==idArray[i])
            {
                find = true;
                break;
            }
        }
        if(find==true)
        {
            break;
        }
        //go step again
        tk = m_Tokenizer.GetToken();
        id = tk->type_id();
    };

}
/** for () xxx  ;
         ^
or
for () {xxx;}
     ^
*/
void ParserThread::SkipStatementBlock()
{
    RawToken * tk = m_Tokenizer.GetToken();
    QUEX_TYPE_TOKEN_ID id = tk->type_id();
    printf("Skip statement block Start line(%d) column(%d)\n",tk->line_number(),tk->column_number());
    if (id == TKN_L_BRACE)
    {
        //SkipToId(TKN_R_BRACE);
        int level = 1;

        while(level>0)
        {
            tk = m_Tokenizer.GetToken();
            id = tk->type_id();
            if (id == TKN_L_BRACE)
                level++;
            else if (id == TKN_R_BRACE)
                level--;
        }
    }
    else
    {
        //SkipToId(TKN_SEMICOLON);
        id = tk->type_id();
        while(id!=TKN_SEMICOLON)
        {
            tk = m_Tokenizer.GetToken();
            id = tk->type_id();
            if (id == TKN_L_BRACE)  // {     ---> skip { xxx } block
            {
                //SkipToId(TKN_R_BRACE);
                int level = 1;

                while(level>0)
                {
                    tk = m_Tokenizer.GetToken();
                    id = tk->type_id();
                    if (id == TKN_L_BRACE)
                        level++;
                    else if (id == TKN_R_BRACE)
                        level--;
                }
            }
        }
    }
    printf("Skip statement block End line(%d) column(%d)\n",tk->line_number(),tk->column_number());
}

void ParserThread::SkipParentheses()
{
    //TKN_L_PAREN

    RawToken * tk = m_Tokenizer.GetToken();//remove the first TKN_L_PAREN
    QUEX_TYPE_TOKEN_ID id = tk->type_id();
    int level = 1;
    printf("Skip Parentheses Start at line(%d)column(%d)\n",tk->line_number(),tk->column_number());
    while(level>0)
    {
        tk = m_Tokenizer.GetToken();
        id = tk->type_id();
        if (id == TKN_L_PAREN)
            level++;
        else if (id == TKN_R_PAREN)
            level--;
    }
    printf("Skip Parentheses End at line(%d)column(%d)\n",tk->line_number(),tk->column_number());

}

void ParserThread::ReadFunctionArguments(ArgumentList &args)
{
    //TKN_L_PAREN
    args.clear();

    RawToken * tk = m_Tokenizer.GetToken();//remove the first TKN_L_PAREN

    QUEX_TYPE_TOKEN_ID id = tk->type_id();

    int level = 1;
    printf("Skip Parentheses Start at line(%d)column(%d)\n",tk->line_number(),tk->column_number());
    while(level>0)
    {
        tk = m_Tokenizer.GetToken();
        id = tk->type_id();
        if (id == TKN_L_PAREN)
            level++;
        else if (id == TKN_R_PAREN)
            level--;
        else
            args<<tk->get_text() ;
    }
    printf("Skip Parentheses End at line(%d)column(%d)\n",tk->line_number(),tk->column_number());

}
void ParserThread::SkipBlock()
{

}

void ParserThread::SkipAngleBraces()
{

}

bool ParserThread::ParseBufferForUsingNamespace(const cc_string& buffer, vector<cc_string>& result)
{
    return true;
}

bool ParserThread::InitTokenizer()
{
    if (!m_Buffer.empty())
    {
        if (!m_IsBuffer)
        {
            m_Filename = m_Buffer;
            m_FileSize = wxFile(m_Filename).length();

            TRACE(cc_text("InitTokenizer() : m_Filename='%s', m_FileSize=%d."), m_Filename.c_str(), m_FileSize);

            bool ret = m_Tokenizer.Init(m_Filename, m_Options.loader);
            delete m_Options.loader;
            m_Options.loader = 0;
            return ret;
        }

        return m_Tokenizer.InitFromBuffer(m_Buffer);
    }
    return false;
}

bool ParserThread::Parse()
{
    TRACE(cc_text("Parse() : parsing %s"),m_Filename.c_str());
    if (!InitTokenizer())
        return false;
    bool result = false;

    m_Context.inTypedef = false;
    m_Context.parentToken = 0;

    do
    {
        if (!m_pTokensTree || !m_Tokenizer.IsOK())
            break;

        if (!m_Options.useBuffer)   // Parse a file
        {
            s_MutexProtection.Enter();
            m_FileIdx = m_pTokensTree->ReserveFileForParsing(m_Filename);
            s_MutexProtection.Leave();
            if (!m_FileIdx)
                break;
        }
        try
        {
            DoParse();
        }
        catch(ParserException& e)
        {
            cout<< "end of file" <<endl;
        }


        if (!m_Options.useBuffer)   // Parsing a file
        {
            s_MutexProtection.Enter();
            m_pTokensTree->FlagFileAsParsed(m_Filename);
            s_MutexProtection.Leave();
        }
        result = true;
    }
    while(false);

    return result;
}

void ParserThread::DoParse()
{

    m_Tokenizer.RunTest();
    return;
//    TestFunction();
//    return;


    while (m_Tokenizer.NotEOF())
    {
        RawToken* tk = GetToken();

        switch (tk->type_id())
        {
        case TKN_L_BRACE : //{
        {
            int idArray[1] = {TKN_R_BRACE};
            SkipToOneOfId(idArray,1);
            TRACE("Skiping {}");
            break;
        }
        case TKN_R_BRACE: //}
        {


            // the only time we get to find a } is when recursively called by e.g. HandleClass
            // we have to return now...
            TRACE("return from a }");
            return;
        }
        case TKN_L_PAREN :       // (
        {
            int idArray[1] = {TKN_R_PAREN};
            SkipToOneOfId(idArray,1);
            TRACE("Skiping ()");
            break;
        }
        case TKN_FOR:
        case TKN_WHILE:
        case TKN_SWITCH:
        case TKN_IF:
        case TKN_ELSE:
        {
            TRACE("handling for or while block");
            SkipParentheses();
            SkipStatementBlock();
            break;
        }
        case TKN_CLASS:
        {
            m_Context.EndStatement();
            if (m_Options.handleClasses)
                HandleClass(ctClass);
            else
                SkipStatementBlock();
            break;
        }


        case  TKN_EXPLICIT:
        case  TKN_CONST:
        case  TKN_FRIEND:
        case  TKN_VIRTUAL:
        case  TKN_VOLATILE:
        case  TKN_STATIC:
        case  TKN_INLINE:
        case  TKN_AUTO:
        case  TKN_REGISTER:
        case  TKN_MUTABLE:
        {
            //m_Context.typeQualifier = tk->type_id();
            break;
        }
        case TKN_IDENTIFIER:
        {

            ParseFullIdentifer();
            // we are interested in the following token.

            RawToken * peek = PeekToken();
            int peekID = peek-> type_id();
            switch (peekID)
            {
            case TKN_L_PAREN:      // This is a function definition or declration, because it has AAA BBB (

                if(m_Context.name.empty())   // AAA(
                {
                    //HandleMacro();
                    SkipParentheses();
                }
                else                            // AAA BBB(
                {
                    cc_string functionName;
                    // the last element of the m_Context.name is the actual function name
                    functionName = m_Context.name.back().name.get_text();

                    HandleFunction(functionName);
                }
                break;
            case  TKN_SEMICOLON:     // A B;
                if(m_Context.name.empty())   // AAA;
                {
                    SkipStatementBlock();
                }
                else                            // AAA BBB;
                {
                    // the last element of the m_Context.name is the actual variable name
                    cc_string variableName = m_Context.name.back().name.get_text();

                    // the last element of the m_Context.type is the actual type name
                    cc_string variableType = m_Context.type.back().name.get_text();

                    TRACE("Variable Find name(%s) type(%s)",variableName.c_str(),variableType.c_str());
                    //Add variable to tokenstree
                    int line = m_Context.name.back().name.line_number();
                    if (m_Options.handleVars)
                        DoAddToken(tkVariable, variableName, line);


                    GetToken();    //consume the semicolon
                    m_Context.EndStatement();
                }
                break;
            case TKN_BITAND://&
                m_Context.typeQualifier.isReference = true;
                GetToken();
                break;
            case TKN_MULT:  //*
                m_Context.typeQualifier.isPointer = true;
                GetToken();
                break;
            case TKN_L_SQUARE:
            case TKN_PLUS:
            case TKN_PLUS_ASSIGN:
            case TKN_DOUBLE_PLUS:
            case TKN_MINUS:
            case TKN_DOUBLE_MINUS:
            case TKN_MINUS_ASSIGN:
            case TKN_MULT_ASSIGN:
            case TKN_DIV:
            case TKN_DIV_ASSIGN:
            case TKN_MODULO:
            case TKN_MODULO_ASSIGN:
            case TKN_L_SHIFT:
            case TKN_L_SHIFT_ASSIGN:
            case TKN_R_SHIFT:
            case TKN_R_SHIFT_ASSIGN:
                GetToken();
                SkipStatementBlock();
                break;

            case TKN_COMMA:
            case TKN_ASSIGN:
                // A B = ....;
                // A B,C,D=2;
                if(m_Context.name.empty())   // A,
                {
                    TRACE("Something wrong in mode A, ");
                    SkipStatementBlock();
                }
                else                            // A B,
                {
                    cc_string variableName = m_Context.name.back().name.get_text();
                    cc_string variableType = m_Context.type.back().name.get_text();
                    TRACE("Variable Find name(%s) type(%s)",variableName.c_str(),variableType.c_str());
                    //Add variable to tokenstree
                    int line = m_Context.name.back().name.line_number();
                    if (m_Options.handleVars)
                        DoAddToken(tkVariable, variableName, line);
                    GetToken();       //consume , or =
                    // tk is updated
                    if(tk->type_id()  == TKN_ASSIGN)
                    {
                        int idArray[2] = {TKN_COMMA,TKN_SEMICOLON};
                        SkipToOneOfId(idArray,2);

                        if(tk->type_id() == TKN_SEMICOLON)
                            m_Context.EndStatement();
                    }

                    m_Context.name.clear();  // clear name information, because we need to read another name

                }
                break;
            }
            break;
        }// End of handling of case TKN_IDENTIFIER
        case TKN_SEMICOLON:
        {
            m_Context.EndStatement();
            break;
        }
        case TKN_PUBLIC:
        {
            RawToken * peek = PeekToken();
            if(peek->type_id() == TKN_COLON)
            {
                m_Context.accessScope = tsPublic;
                m_Context.EndStatement();
                GetToken();
            }
            break;
        }
        case TKN_PRIVATE:
        {
            RawToken * peek = PeekToken();
            if(peek->type_id() == TKN_COLON)
            {
                m_Context.accessScope = tsPrivate;
                m_Context.EndStatement();
                GetToken();
            }
            break;
        }
        case TKN_PROTECT:
        {
            RawToken * peek = PeekToken();
            if(peek->type_id() == TKN_COLON)
            {
                m_Context.accessScope = tsProtected;
                m_Context.EndStatement();
                GetToken();
            }
            break;
        }
        case TKN_USING:
        {
            RawToken * peek = PeekToken();
            if(peek->type_id() == TKN_NAMESPACE)
            {
                //Handleing using space directive
                GetToken(); //consume "namespace"
                // simply handling mode: using namespace AAA
                tk = GetToken();
                if (tk->type_id() == TKN_IDENTIFIER)
                    DoAddToken(tkUsingNamespace,tk->get_text(),tk->line_number());
                else
                    SkipStatementBlock();

            }
            break;
        }
        case TKN_DELETE:
        case TKN_NEW:
        case TKN_RETURN:
        case TKN_EXTERN:
        case TKN_GOTO:
        {
            SkipStatementBlock();
            break;
        }
        case TKN_TEMPLATE:
        {
            bool readArgsOK = GetTemplateArgs();   // should following a <> pair,
            if(!readArgsOK);   //should return true, otherwise, we should
                SkipStatementBlock();
            break;
        }
        case TKN_ENUM:
        {
            if (m_Options.handleEnums)
                HandleEnum();
            else
                SkipStatementBlock();
            break;
        }
        case TKN_TYPEDEF:
        {
            if (m_Options.handleTypedefs)
                HandleTypedef();
            else
                SkipStatementBlock();
            m_Context.EndStatement();

            break;
        }
        default:
        {
            //cout<<"Skip unhandled"<<*tk<<endl;
            // As the tk is only a pointer to the RawToken buffer, so it may changed by
            // some function calling on GetToken or PeekToken
            break;
        }
        }
    }

}

Token* ParserThread::TokenExists(const cc_string& name, Token* parent, short int kindMask)
{
    Token* result;
    if (!m_pTokensTree)
        return 0;
    int parentidx = !parent ? -1 : parent->GetSelf();
    // no critical section needed here:
    // all functions that call this, already entered a critical section.
    result = m_pTokensTree->at(m_pTokensTree->TokenExists(name, parentidx, kindMask));
    return result;
}

cc_string ParserThread::GetActualTokenType()
{
    // we will compensate for spaces between
    // namespaces (e.g. NAMESPACE :: SomeType) wich is valid C++ construct
    // we 'll remove spaces that follow a semicolon
//    int pos = 0;
//    while (pos < (int)m_Str.length())
//    {
//        if (m_Str.at(pos) == ' ' &&
//            (
//                (pos > 0 && m_Str.at(pos - 1) == ':') ||
//                (pos < (int)m_Str.length() - 1 && m_Str.at(pos + 1) == ':')
//            )
//           )
//        {
//            m_Str.erase(pos, 1);
//        }
//        else
//            ++pos;
//    }
//
//    // m_Str contains the full text before the token's declaration
//    // an example m_Str value would be: const string&
//    // what we do here is locate the actual return value (string in this example)
//    // it will be needed by code completion code ;)
//    pos = m_Str.length() - 1;
//    // we walk m_Str backwards until we find a non-space cc_character which also is
//    // not * or &
//    //                        const string&
//    // in this example, we would stop here ^
//    while (pos >= 0 &&
//            (isspace(m_Str.at(pos)) ||
//            m_Str.at(pos) == '*' ||
//            m_Str.at(pos) == '&'))
//        --pos;
//    if (pos >= 0)
//    {
//        // we have the end of the word we 're interested in
//        int end = pos;
//        // continue walking backwards until we find the start of the word
//        //                               const string&
//        // in this example, we would stop here ^
//        while (pos >= 0 && (isalnum(m_Str.at(pos)) || m_Str.at(pos) == '_' || m_Str.at(pos) == ':'))
//            --pos;
//        return m_Str.substr(pos + 1, end - pos);
//    }
//    else
//        return m_Str; // token ends at start of phrase
    return cc_text(""); // never reaches here
}

cc_string ParserThread::GetQueueAsNamespaceString(std::queue<cc_string>& q)
{
    cc_string result;
    while (!q.empty())
    {
        //result << q.front() << ParserConsts::dcolon;
        q.pop();
    }
    return result;
}

Token* ParserThread::FindTokenFromQueue(std::queue<cc_string>& q, Token* parent, bool createIfNotExist, Token* parentIfCreated)
{
    if (q.empty())
        return 0;
    cc_string ns = q.front();
    q.pop();

    Token* result = TokenExists(ns, parent, tkNamespace | tkClass);

    if (!result && createIfNotExist)
    {
        result = new Token(ns, m_FileIdx, 0);
        result->m_TokenKind = q.empty() ? tkClass : tkNamespace;
        result->m_IsLocal = m_IsLocal;
        result->m_ParentIndex = parentIfCreated ? parentIfCreated->GetSelf() : -1;
        int newidx = m_pTokensTree->insert(result);
        if (parentIfCreated)
            parentIfCreated->AddChild(newidx);

        TRACE(cc_text("FindTokenFromQueue() : Created unknown class/namespace %s (%d) under %s (%d)"),
              ns.c_str(),
              newidx,
              parent ? parent->m_Name.c_str() : cc_text("<globals>"),
              parent ? parent->GetSelf() : -1);
    }

    if (q.empty())
        return result;
    if (result)
        result = FindTokenFromQueue(q, result, createIfNotExist, parentIfCreated);
    return result;
}

static cc_string GetRealArgs(const cc_string & args)
{
    const cc_char * ptr = args.c_str();
    cc_string str;
    cc_string word;
    bool skip = false; // next cc_char skip
    bool sym  = false; // cur cc_char symbol
    while (*ptr != cc_text('\0'))
    {
        switch (*ptr)
        {
        case cc_text(' '):
            if (word == cc_text("const") || word == cc_text("volatile") )
                skip = false;
            else
                skip = true;
            word = cc_text("");
            sym  = false;
            break;
        case cc_text('*'):
        case cc_text('&'):
            word = cc_text("");
            skip = true;
            sym  = true;
            // test
            {
                int comma = str.rfind(cc_text(','));
                int begin = 1;
                if (comma != -1)
                    begin = comma;
                if ( str.substr(begin).find(cc_text('('),begin) == string::npos)
                {
                    str += *ptr;
                    //find end
                    int n = 0;
                    while(*ptr != cc_text('\0'))
                    {
                        ptr++;
                        if (*ptr == cc_text('('))
                            n++;
                        else if (*ptr == cc_text(')'))
                        {
                            if (n == 0)
                                break;
                            n--;
                        }
                        else if (*ptr == cc_text(','))
                        {
                            sym = true;
                            skip = false;
                            break;
                        }
                    }
                }
            }
            break;
        case cc_text(':'):
            skip = false;
            sym  = true;
            break;
        case cc_text('['):
            while (*ptr != cc_text('\0') && *(ptr) != cc_text(']'))
            {
                if (*ptr != cc_text(' '))
                    str += *ptr;
                *ptr++;
            }
            skip = true;
            sym  = true;
            break;
        case cc_text('<'):
            while (*ptr != cc_text('\0') && *(ptr) != cc_text('>'))
            {
                if (*ptr != cc_text(' '))
                    str += *ptr;
                *ptr++;
            }
            skip = true;
            sym  = true;
            break;
        case cc_text(','):
        case cc_text(')'):
        case cc_text('('):
            word = cc_text("");
            sym  = true;
            skip = false;
            break;
        default:
            sym = false;
        }
        if (!skip || sym)
        {
            str += *ptr;
            if (isalnum(*ptr))
                word += *ptr;
        }

        if (sym == true && skip == false)
        {
            while (*ptr != cc_text('\0') && *(ptr+1) == cc_text(' '))
                ptr++;
        }
        ptr++;
    }

    return str;
}

Token* ParserThread::DoAddToken(TokenKind kind,
                                const cc_string& name,
                                int line,
                                int implLineStart,
                                int implLineEnd,
                                const cc_string& args,
                                bool isOperator,
                                bool isImpl)
{

    if (name.empty())
        return 0; // oops!

    s_MutexProtection.Enter();
    Token* newToken = 0;
    cc_string newname(name);
    Token * finalParent = m_Context.parentToken;

    newToken = new Token(newname,m_FileIdx,line);
    newToken->m_ParentIndex = finalParent ? finalParent->GetSelf() : -1;
    newToken->m_TokenKind = kind;

    int newidx = m_pTokensTree->insert(newToken);
    if (finalParent)
        finalParent->AddChild(newidx);

    TRACE("Adding Token name(%s)line(%d)",newname.c_str(),line);

    newToken->m_FullType = m_Context.type;
    newToken->m_FullName = m_Context.name;

    if(!m_Context.type.empty())
    {
        newToken->m_Type = m_Context.type.back().name.get_text();

    }
    newToken->m_FileIdx = m_FileIdx;
    newToken->m_Line    = line;
    TRACE(cc_text("DoAddToken() : Added/updated token '%s' (%d), type '%s', actual '%s'. Parent is %s (%d)"),
          name.c_str(),                   newToken->GetSelf(),                newToken->m_Type.c_str(),
          newToken->m_ActualType.c_str(), newToken->GetParentName().c_str(), newToken->m_ParentIndex);



    s_MutexProtection.Leave();
    return newToken;
}

void ParserThread::HandleIncludes()
{

}

void ParserThread::HandleDefines()
{

}

void ParserThread::HandleNamespace()
{

}

void ParserThread::ReadVarNames()
{

}

void ParserThread::HandleClass(EClassType ct)
{

    // class xxx {   or class {
    // the keyworkd "class" is already comsumed
    assert(CurrentToken()->type_id()==TKN_CLASS);


    RawToken * current =  GetToken();      // class name
    RawToken * next    =  PeekToken();

    TRACE("HandleClass() : Found class '%s'", current->get_text().c_str() );


    // Check current firstly
    if (current->type_id() == TKN_L_BRACE)   // unnamed class/struct/union
    {
//                cc_string unnamedTmp;
//                unnamedTmp.Printf(_T("%s%s%d"),
//                                  ParserConsts::unnamed.wx_str(),
//                                  ct == ctClass ? _T("Class") :
//                                  ct == ctUnion ? _T("Union") :
//                                                  _T("Struct"), ++m_pTokensTree->m_StructUnionUnnamedCount);
//
//                Token* newToken = DoAddToken(tkClass, unnamedTmp, lineNr);
//
//                PushContext();
//                m_Context.Clear();
//                m_Context.parentToken = newToken;
//                m_Context.accessScope  = ct == ctClass ? tsPrivate : tsPublic; // struct has default public scope
//
//				newToken->m_ImplLine = lineNr;
//				newToken->m_ImplLineStart = m_Tokenizer.GetLineNumber();
//
//                DoParse();
//
//                PopContext();
    }
    else if (current->type_id() == TKN_IDENTIFIER)     //OK, we need to check the next
    {
        if ( next->type_id() == TKN_L_BRACE)   // class AAA {, we find the "{" here
        {
            Token* newToken = DoAddToken(tkClass, current->get_text(), current->line_number());
            ParserThreadContext savedContext = m_Context;
            m_Context.EndStatement();
            m_Context.parentToken = newToken;
            GetToken();// consume {
            DoParse();  // when meet a }, we should return from DoParse()
            m_Context = savedContext;
            assert(CurrentToken()->type_id()==TKN_R_BRACE);

            current = GetToken();
            if(current->type_id()==TKN_SEMICOLON)  // class A {.....};
                return;
            else
                SkipStatementBlock(); //struct A {....} a b;

        }

    }
    else
    {
        // something wrong, should be skip to a semicolon
        SkipStatementBlock();
    }



    // restore tokenizer's functionality

}

void ParserThread::HandleFunction(cc_string & name)
{
    TRACE("HandleFunction() : %s %s()",m_Context.type.back().name.get_text().c_str(),name.c_str());
    ArgumentList args;
    ReadFunctionArguments(args);

    //SkipParentheses();  //parameter

    RawToken * peek = m_Tokenizer.PeekToken();

    if (peek->type_id() == TKN_L_BRACE)   // function definition
    {
        TRACE("Function definition");
        TokenKind tokenKind = tkFunction ;
        cc_string functionName(name);
        Token* newToken =  DoAddToken(tokenKind, functionName,peek->line_number());
        newToken->m_Args = args;
        SkipStatementBlock();
    }
    else if (peek->type_id() == TKN_SEMICOLON)
    {
        TRACE("Function declaration");
        TokenKind tokenKind = tkFunction ;
        cc_string functionName(name);
        Token* newToken =  DoAddToken(tokenKind, functionName,peek->line_number());
        newToken->m_Args = args;
        m_Tokenizer.GetToken();
    }
    m_Context.EndStatement();
}

void ParserThread::ReadEnumList()
{
    RawToken *current;
    RawToken *peek;
    do
    {
        current = GetToken();
        peek    = PeekToken();
        if(current->type_id() == TKN_IDENTIFIER )
        {
            if (peek->type_id() == TKN_ASSIGN)     // a = b,
            {
                //Add variable
                DoAddToken(tkVariable, current->get_text(), current->line_number());
                GetToken(); //comsume "="
                GetToken(); //comsume the id after "="
            }
            else if (peek->type_id() == TKN_R_BRACE)  //a}
            {
                //Add variable
                DoAddToken(tkVariable, current->get_text(), current->line_number());
                break;
            }
            else if (peek->type_id() == TKN_IDENTIFIER)      // id p1 p2
            {
                RawToken * peek2 = PeekToken(2);
            }
            else if (peek->type_id() == TKN_COMMA)     // a,....
            {
                //Add variable
                DoAddToken(tkVariable, current->get_text(), current->line_number());
                GetToken(); //comsume ","
            }
        }
    }
    while(peek->type_id() != TKN_R_BRACE);

}
void ParserThread::HandleEnum()
{
    // enums have the following rough definition:
    // enum [xxx] { type1 name1 [= 1][, [type2 name2 [= 2]]] };


    RawToken * tk = GetToken();    // the token after "enum"
    Token * newToken = 0;
    if (tk->type_id() == TKN_IDENTIFIER)           // enum XXX
    {
        RawToken *pk = PeekToken();
        if(pk->type_id() == TKN_L_BRACE )  //enum XXX {
        {
            // Do Add Token of enum
            TRACE("find enum %s at line(%d)",tk->get_text().c_str(),tk->line_number());
            newToken = DoAddToken(tkEnum, tk->get_text(), tk->line_number());
            newToken->m_ImplLineStart = pk->line_number();
            GetToken();                     //consume {
            ParserThreadContext savedContext = m_Context;
            m_Context.parentToken = newToken;
            ReadEnumList();
            m_Context = savedContext;
        }
        else
        {
            SkipStatementBlock();          //something wrong
            return;
        }
    }
    else if (tk->type_id()== TKN_L_BRACE) //  enum {
    {
        // // Do Add Token of enum
        TRACE("find unnamed enum at line(%d)",tk->line_number());
        newToken = DoAddToken(tkEnum, "UnnamedEnum", tk->line_number());
        GetToken();                    //consume {
        ParserThreadContext savedContext = m_Context;
        m_Context.parentToken = newToken;
        ReadEnumList();
        m_Context = savedContext;
    }

    // Token's implement End line information added
    tk = GetToken();
    //we are now after the }
    if(newToken)
    {
        newToken->m_ImplLineEnd = tk->line_number();
    }

    // if we find a ;, good, end of definition of enum
    // if we find an id, then this is something like enum XXX{...} A,B;
    // A,B were the instant of the enum

    tk = GetToken();
    if(tk->type_id() == TKN_IDENTIFIER)
    {
        //Add variables list
        return;
    }
    else if(tk->type_id() == TKN_SEMICOLON)
    {
        //OK, end of enum definition
        return;
    }

}

void ParserThread::HandleTypedef()
{

    TRACE("Typedef find");
    SkipStatementBlock();
}

void ParserThread::ReadClsNames(cc_string& ancestor)
{

}

void ParserThread::HandleMacroUsage(const cc_string &token, const cc_string &peek)
{

}

void ParserThread::PushContext()
{

}
void ParserThread::PopContext()
{

}

bool ParserThread::GetTemplateArgs()
{
    m_Context.templateArgument.clear();
    int nestLvl = 0;

    while (true)
    {
        RawToken * tk = GetToken();

        if (tk->type_id() == TKN_LESS) // <
        {
            ++nestLvl;
            //m_Context.templateArgument << tk->get_text() << " ";

        }
        else if (tk->type_id() == TKN_GREATER) // >
        {
            --nestLvl;
            //m_Context.templateArgument << tk->get_text() << " ";
        }
        else if (tk->type_id()==TKN_SEMICOLON) // ;
        {
            // unget token - leave ; on the stack
            m_Context.templateArgument.clear();
            return false;
        }
        else if( tk->type_id() == TKN_COMMA)
        {
            m_Context.templateArgument << ",";
        }
        else
            m_Context.templateArgument << tk->get_text() << " ";

        if (nestLvl <= 0)
        {
            TRACE("ParserThread::GetTemplateArgs() %s", m_Context.templateArgument.c_str());
            break;
        }

    }
    return true;

}


void ParserThread::HandlePreprocessorDirective(RawToken * tk)
{
    if( TKN_PP_DEFINE <= tk->type_id() && tk->type_id() <= TKN_PP_ERROR)
    {
        //loop until we find a TKN_TKN_PP_FINISH
        switch (tk->type_id())
        {

        case TKN_PP_DEFINE:
            {
                tk = m_Tokenizer.GetToken();
                cout<<"Macro definition name="<<tk->get_text()<<endl; //should be an ID
                // read the macro definition.
                RawToken* peek = PeekToken();
                if(peek->type_id()==TKN_L_PAREN) // this is a function like macro definition
                {
                    SkipParentheses();
                }
                do
                {
                    tk = m_Tokenizer.GetToken();
                    cout<<*tk<<endl; //should be an ID

                }while(TKN_PP_FINISH != tk->type_id());
                break;
            }
        case TKN_PP_IF:
            {
                break;
            }
        case TKN_PP_ELIF:
            {
                break;
            }
        case TKN_PP_IFDEF :
            {
                break;
            }
        case TKN_PP_IFNDEF:
            {
                break;
            }
        case TKN_PP_ENDIF:
            {
                break;
            }
        case TKN_PP_ELSE:
            {
                break;
            }
        case TKN_PP_PRAGMA:
            {
                break;
            }
        case TKN_PP_ERROR:
            {
                break;
            }
        case TKN_PP_UNDEF:
            {
                break;
            }

        }


    }
}


bool ParserThread::ParseFullIdentifer()
{
    if(m_Context.type.size()==0)
        ParseScopeQueue(m_Context.type);
    else if (m_Context.name.size()==0)
        ParseScopeQueue(m_Context.name);
    else { // both type and name has values, so we push the name to type
        m_Context.type = m_Context.name;
        // fill the name again
        ParseScopeQueue(m_Context.name);
    }
    return true;
}
bool ParserThread::ParseScopeQueue(FullIdentifier& scopeQueue)
{
    RawToken * currentToken = CurrentToken();
    assert(currentToken->type_id() == TKN_IDENTIFIER);

    ScopeBlock scope;
    scope.name = *currentToken;// add the name
    ParseArgumentList(scope.templateArgumentList);
    scopeQueue.push_back(scope);

    while(PeekToken()->type_id()==TKN_DOUBLE_COLON && PeekToken(2)->type_id()==TKN_IDENTIFIER)
    {
            //scope.name.Clear(); should reset the Token
            scope.templateArgumentList.clear();
            //Read the Argument list
            // fill the *it.templateArgumentList
            // if the fill function return false, which, we should skip to ; or }
            GetToken(); //consume ::
            currentToken = GetToken(); // get id
            scope.name = *currentToken;// add the name
            bool parseResult = ParseArgumentList(scope.templateArgumentList);
            if (!parseResult) // this means we have the format A<......;, this is not a template
            {
                return false;
            }
            scopeQueue.push_back(scope);
    }
    return true;

}
bool ParserThread::ParseArgumentList(ArgumentList &argumentList)
{
    // do nothing on the argumentList, just do the loop
    if(PeekToken()->type_id() != TKN_LESS)
       return true;

    // shoud be a (xxxx) or <xxxxx>
    RawToken * tk = GetToken();
    std::cout<<"ParserThread::ParseArgumentList() Enter...\n";
    if(tk->type_id() == TKN_LESS)
    {
        int level = 1;
        argumentList.clear();
        argumentList<<tk->get_text(); // push <
        tk = GetToken();// get a token
        // should match a >
        while(tk->type_id()!=TKN_GREATER)
        {
            if(tk->type_id()==TKN_SEMICOLON or tk->type_id()==TKN_R_BRACE)
            {
                m_Context.EndStatement();
                return false;
            }
            std::cout<<*tk<<std::endl;
            argumentList<<tk->get_text();
            tk = GetToken();// comsume this one;

        }
        assert(tk->type_id() == TKN_GREATER);
    }

    cout<<"currentToken"<<*(CurrentToken())<<endl;
    std::cout<<"ParserThread::ParseArgumentList() Leave...\n";
    return true;
}


void ParserThread::TestFunction()
{
    while (m_Tokenizer.NotEOF())
    {
        RawToken* tk = GetToken();
        std::cout<<*tk<<std::endl;
        int type = tk->type_id();
        switch (type)
        {
        case TKN_IDENTIFIER:
        {
            ParseFullIdentifer();
            break;
        }
        case TKN_SEMICOLON:
        {
            cout<<"Meet SemiColon,Now reset the statement context!"<<endl;
            m_Context.Dump();
            m_Context.EndStatement();
        }
        default:
            break;
        }

    }
}
