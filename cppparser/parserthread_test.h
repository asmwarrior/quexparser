/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H

#include <vector>
using namespace std;

#include "preprocessor.h"

class Token
{

};

enum TokenScope
{
    tsUndefined = 0,
    tsPrivate,
    tsProtected,
    tsPublic
};

enum TokenKind
{

    tkUndefined        = 0,
    tkNamespace           ,
    tkClass               ,
    tkTemplateClass       ,
    tkEnum                ,
    tkTypedef             ,
    tkConstructor         ,
    tkDestructor          ,
    tkFunction            ,
    tkTemplateFunction    ,
    tkVariable            ,
    tkEnumerator          ,
    tkMacroDefine         ,
    tkMacroUsage 	      ,
    tkUsingNamespace

};

struct DeclarationModifier
{
    bool isConst;
    bool isConstructor;
    bool isDestructor;
    bool isVirtual;
    bool isOperator;
    bool isTemplate;
    bool isAuto;
    bool isRegister;
    bool isExtern;
    bool isExplicit;
    bool isFriend;
    bool isVolatile;
    bool isStatic;
    bool isInline;
    bool isMutable;
    bool isPointer;
    bool isReference;
};

/** one argument
  * for example: void f(int a=0, float b=3);
  * then there are two SingleArgument
  * "int   a 0"
  * "float b 3"
  */
struct SingleArgument
{
    RawToken type;
    RawToken name;
    RawToken defaultValue;
};
typedef std::vector<RawToken> ArgumentList;

/** for a template type definition, we have such format
  *   AAA<typename T1 = int, class T2 = float>::
  */
struct ScopeBlock
{
    RawToken     name;
    ArgumentList templateArgumentList;
};

/** AAA<typename T1 = int, class T2 = float>::BBB */
typedef std::vector<ScopeBlock> ScopeQueue;


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
        ScopeQueue::iterator it = type.begin();
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
    ScopeQueue name;

    /** type information of a function or a variable
      * for a function declaration below:
      * ClassC::returnValue ClassA::FunctionB();
      * type =  ClassC::returnValue
      */
    ScopeQueue type;

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
    vector<RawToken>  templateArgument;

    /** type qualifier like: const, static.... */
    DeclarationModifier   typeQualifier;


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
    ParserThread(const cc_string& bufferOrFilename);

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


    /** handle function declaration or definition
      * @param name is the function name
      * @param isOperator if true, means it is a operator override function */
    void HandleFunction();

    /** handle enum declaration */
    void HandleEnum();

    /** handle typedef directive */
    void HandleTypedef();

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
    Token *DoAddToken(TokenKind kind,
                      const cc_string& name,
                      int line);

    // consume a token
    inline RawToken * GetToken()
    {
        if (TestDestroy())
        {
            throw ParserException();
        }
        RawToken *tk = m_Preprocessor.GetToken();
        return tk;

    }

    // peek a token, the default step was 1 (the next after the current one)
    inline RawToken * PeekToken(int step = 1)
    {
        if (TestDestroy())
        {
            throw ParserException();
        }
        return m_Preprocessor.PeekToken(step);
    }
    // unget token
    inline void  UngetToken()
    {
        if (TestDestroy())
        {
            throw ParserException();
        }
        m_Preprocessor.UngetToken();
    }

    bool ParseFullIdentifer();
    bool ParseScopeQueue(ScopeQueue& scopeQueue);
    bool ParseArgumentList(ArgumentList &argumentList);
    void TestFunction();


    /** if we regard the parserThread class as a syntax analyzer, then the Tokenizer class is
      * regard as the lexer, which always return a string by calling m_Tokenizer.GetToken() */
    Preprocessor          m_Preprocessor;

    ParserThreadOptions m_Options;

    ParserThreadContext m_Context;

    std::stack<ParserThreadContext> m_ContextStack;

};

#endif // PARSERTHREAD_H
