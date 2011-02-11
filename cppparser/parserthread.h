/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H


#include "tokenizer.h"
#include "token.h"

//extern int THREAD_START;
//extern int THREAD_END;
extern int NEW_TOKEN;
extern int FILE_NEEDS_PARSING;

class Parser;
static wxCriticalSection s_mutexListProtection;

struct ParserThreadContext
{

    ParserThreadContext():
        accessScope(tsUndefined) ,
        inTypedef(false)
    {

    };


    void EndStatement()
    {
        type.clear();
        name.clear();
    }

    void Dump()
    {
        cout<<"Dump Context...\n";
        cout<<"Type information list\n";
        FullIdentifier::iterator it = type.begin();
        for(;it!=type.end();it++)
        {
            cout<<(*it).name<< " ";
            ArgumentList::iterator it2 = (*it).templateArgumentList.begin();
            for(;it2!=(*it).templateArgumentList.end();it2++)
            {
                cout<<*it2<< " ";
            }
            cout<<endl;
        }
        cout<<"Name information list\n";
        it = name.begin();
        for(;it!=name.end();it++)
        {
            cout<<(*it).name<< " ";
            ArgumentList::iterator it2 = (*it).templateArgumentList.begin();
            for(;it2!=(*it).templateArgumentList.end();it2++)
            {
                cout<<*it2<< " ";
            }
            cout<<endl;
        }

    }

    /** this is a very important member variables! it serves as a return type stack,
      * eg: string const varA;
      * in this statement, we should step until the last ';" to determine this
      * is a Token named 'varA', every token before 'varA' will be pushed to 'type', see below
      */
    FullIdentifier name;

    /** type information of a function or a variable
      * for a function declaration below:
      * ClassC::returnValue ClassA::FunctionB();
      * type =  ClassC::returnValue
      */
    FullIdentifier type;

    /** parent Token, for example, you are parsing in a class declearation, then this member
      * keep the pointer to the current class Token
      */
    Token* parentToken;

    /** this member define the scope type of member variables, which is: public or private
      * protected or undefined
      */
    TokenScope  accessScope;


    /** this makes a difference in unnamed class/struct/enum handling */
    bool     inTypedef;

    /** temporary record the template argument list information*/
    cc_string   templateArgument;

    /** type qualifier like: const, static.... */
    DeclarationModifier   typeQualifier;

    TokenIdxSet usingNamespaceCollection;

};

struct ParserThreadOptions
{
    ParserThreadOptions() :
        useBuffer(false),
        bufferSkipBlocks(false),
        bufferSkipOuterBlocks(false),
        wantPreprocessor(true),
        followLocalIncludes(true),
        followGlobalIncludes(true),
        isTemp(false),
        handleFunctions(true),
        handleVars(true),
        handleClasses(true),
        handleEnums(true),
        handleTypedefs(true),
        loader(0)
    {}
    /** useBuffer specifies that we're not parsing a file,  but a temporary
      *  buffer. The resulting tokens will be temporary, too,
      * and will be deleted when the next file is parsed.
      */
    bool        useBuffer;
    bool        bufferSkipBlocks;
    bool        bufferSkipOuterBlocks; // classes, namespaces and functions
    bool        wantPreprocessor;
    bool        followLocalIncludes;
    bool        followGlobalIncludes;
    bool        isTemp;

    bool        handleFunctions;
    bool        handleVars;
    bool        handleClasses;
    bool        handleEnums;
    bool        handleTypedefs;

    LoaderBase* loader; // if not NULL, load through filemanager (using threads)
};
/** @brief A parser thread
  *
  * This class represents a thread for the Code Completion plugin.
  */
class ParserThread
{
public:
    bool	TestDestroy()
    {
        return false;
    }
    /** ParserThread constructor.
      * @param parent the parent Parser object which contain the Token Trie
      * @param bufferOrFilename if isLocal is true, it's the filename to open, else it is a
      * string already in memory buffer.
      * @param isLocal determine whether this is a file in local disk or already in memory
      * @param parserThreadOptions parser thread options, see ParserThreadOptions for details.
      * @param tokensTree it is the Trie structure holding all the Tokens, ParserThread will add
      * every Token when it parsed.*/
    ParserThread(Parser* parent,
                 const cc_string& bufferOrFilename,
                 bool isLocal,
                 ParserThreadOptions& parserThreadOptions,
                 TokensTree* tokensTree);

    /** ParserThread destructor.*/
    virtual ~ParserThread();

    /** Execute() is a virtual function in cbThreadedTask class, we should override it here, when
      * this function will be called automatically from the thread pool.*/
    int Execute()
    {
        return Parse() ? 0 : 1;
    }

    /** Do the main job */
    bool Parse();

    /**Get the context using namespace XXX directive
      * @param buffer the buffer string to be parsed
      * @param result the wxArrayString contains all the namespace names*/
    bool ParseBufferForUsingNamespace(const cc_string& buffer, vector<cc_string>& result);

    /** does nothing atm */
    virtual void* DoRun();

    /** never be used*/
    virtual void SetTokens(TokensTree* tokensTree);

    /** return the parsing file name*/
    const cc_string& GetFilename() const
    {
        return m_Filename;
    }
protected:

    /** enum to specify which statement we are handling: struct or class or union*/
    enum EClassType { ctStructure = 0, ctClass = 1, ctUnion = 3 };

    /** skip until we meet one characters in the string
      * @param chars string specifies all the ending characters
      * @param supportNesting when running this function, if supportNesting is true, we need to
      * handle the "{" and "}" nesting levels.*/
    void SkipToOneOfId(const int * idArray, const int num);

    /** actually run the syntax analysis*/
    void DoParse();

    /** skip blocks () [] {} <> */
    void SkipBlock();

    /** skip the template argument*/
    void SkipAngleBraces();

    /** handle include <XXXX>' or include "XXXX" directive, this will internally add another
      * parserThead associate with the included file to parserthead pool*/
    void HandleIncludes();

    /** handle the statement: #define XXXXX  */
    void HandleDefines();

    void ReadFunctionArguments(ArgumentList &args);

    /** handle the statement:
      * namespace XXX {  */
    void HandleNamespace();

    /** eg: class ClassA{...} varA, varB, varC
      * This function will read the "varA, varB, varC" */
    void ReadVarNames();

    /** handle class names
      * @param ancestor define ?? */
    void ReadClsNames(cc_string& ancestor);

    /** handle class declaration
      * @param ct specify type : struct or class */
    void HandleClass(EClassType ct);

    void HandleMacroUsage(const cc_string & token, const cc_string & peek);

    /** handle function declaration or definition
      * @param name is the function name
      * @param isOperator if true, means it is a operator override function */
    void HandleFunction(cc_string & name);

    /** handle enum declaration */
    void HandleEnum();

    /** handle typedef directive */
    void HandleTypedef();

    /** add the Token to the token trie
      * @param kind Token type, see TokenKind Emun for more details
      * @param name Token name, this is the keyword to search in the Token trie
      * @param line line number of the source file
      * @param implLineStart if this is a function implementation, it is the start of function body
      * @param implLineEnd like the one above, it is the end line of the function body
      * @param args if the Token type is a function, then this is the function arguments string
      * @param isOperator an operator override function or not
      * @param isTmpl it is a function declaration or implementation */
    Token* DoAddToken(TokenKind kind,
                      const cc_string& name,
                      int line,
                      int implLineStart = 0,
                      int implLineEnd = 0,
                      const cc_string& args = cc_text(""),
                      bool isOperator = false,
                      bool isImpl = false);

    /** return the actual token type, eg:if the token type string is : "const string &"
      * then, the actual token type is : "string" */
    cc_string GetActualTokenType();

private:

    void SkipStatementBlock();
    void SkipParentheses();
    void PushContext();
    void PopContext();
    bool GetTemplateArgs();
    void ReadEnumList();

    // consume a token
    inline RawToken * GetToken()
    {
        if (TestDestroy())
        {
            throw ParserException();
        }
        RawToken  * tk = m_Tokenizer.GetToken();
        HandlePreprocessorDirective(tk);
        return tk;

    }

    // return the current token pointer
    inline RawToken * CurrentToken()
    {
        return m_Tokenizer.CurrentToken();
    }

    // peek a token, the default step was 1 (the next after the current one)
    inline RawToken * PeekToken(int step = 1)
    {
        if (TestDestroy())
        {
            throw ParserException();
        }
        return m_Tokenizer.PeekToken(step);
    }

    // unget token
    inline void     UngetToken()
    {
        if (TestDestroy())
        {
            throw ParserException();
        }
        m_Tokenizer.UngetToken();
    }

    void HandlePreprocessorDirective(RawToken * tk);
    bool ParseFullIdentifer();
    bool ParseScopeQueue(FullIdentifier& scopeQueue);
    bool ParseArgumentList(ArgumentList &argumentList);
    void TestFunction();

    /** no usage ??? */
    void Log(const cc_string& log);

    /** if parent is 0, all tokens are searched
      * @param name the search key word
      * @param parent parent Token pointer, only search under the parent Token
      * @param kindMask filter for the result Token, which type of Token should we search*/
    Token* TokenExists(const cc_string& name, Token* parent = 0, short int kindMask = 0xFFFF);

    /** NOTICE: clears the queue too*/
    cc_string GetQueueAsNamespaceString(std::queue<cc_string>& q);

    /** to do describe this function */
    Token* FindTokenFromQueue(std::queue<cc_string>& q,
                              Token* parent = 0,
                              bool createIfNotExist = false,
                              Token* parentIfCreated = 0);

    /** if we regard the parserThread class as a syntax analyzer, then the Tokenizer class is
      * regard as the lexer, which always return a string by calling m_Tokenizer.GetToken() */
    Tokenizer            m_Tokenizer;

    /** a pointer to its parent */
    Parser*              m_pParent;

    /** a pointer to the Token trie, all the Tokens will be add to that trie */
    TokensTree*          m_pTokensTree;

    /** the file name of the parsing source */
    cc_string             m_Filename;
    /** file size */
    unsigned int         m_FileSize;

    /** source file index on the Token trie */
    unsigned int         m_FileIdx;

    /** determine whether we are parsing the local files or buffer already in memory */
    bool                 m_IsLocal;





    /** parser options, see the ParserThreadOptions struc */
    ParserThreadOptions  m_Options;




    /**  whether the tokenizer use a buffer */
    bool                 m_IsBuffer;

    /**  buffer string */
    cc_string             m_Buffer;

    /** initialize the m_Buffer member? */
    bool InitTokenizer();

    ParserThreadContext m_Context;
};

#endif // PARSERTHREAD_H
