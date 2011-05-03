#include <iostream>
#include <fstream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include<iomanip>

#include "parserthread_test.h"


#define PARSERTHREAD_DEBUG_OUTPUT 0


#if PARSERTHREAD_DEBUG_OUTPUT

#define TRACE(format, args...)\
    printf(format , ## args);\
    printf("\n")
#else
#define TRACE(format, ...)
#define DebugLog(format,...)
#endif

#define _T(a) a

cc_string GetSymbolKindString(SymbolKind type);

cc_string GetSymbolKindString(SymbolKind type)
{
    switch (type)
    {
        case  tkUndefined       : return _T("");
        case  tkNamespace       : return _T("namespace");
        case  tkClass           : return _T("class");
        case  tkTemplateClass   : return _T("template class");
        case  tkEnum            : return _T("enum");
        case  tkTypedef         : return _T("typedef");
        case  tkConstructor     : return _T("constructor");
        case  tkDestructor      : return _T("destructor");
        case  tkFunction        : return _T("function");
        case  tkTemplateFunction: return _T("template function");
        case  tkVariable        : return _T("variable");
        case  tkEnumerator      : return _T("enumerator");
        case  tkMacroDefine     : return _T("macro define");
        case  tkMacroUsage 	    : return _T("macro usage");
        case  tkUsingNamespace  : return _T("using");
        case  tkFor             : return _T("for");
        case  tkWhile           : return _T("while");
        case  tkTry             : return _T("try");
        case  tkDoWhile         : return _T("do while");
        case  tkParameter       : return _T("parameter");
    }
}



ParserThread::ParserThread(const cc_string& bufferOrFilename)
{
    m_Preprocessor.LoadFile(bufferOrFilename);
    m_Preprocessor.Preprocess();
//    std::cout<<"------------DUMP MACRO----------------------------\n";
//    m_Preprocessor.DumpMacroTable();
//    std::cout<<"------------DUMP Token List----------------------- \n";
//    m_Preprocessor.DumpTokenList();
}

ParserThread::~ParserThread()
{
    //dtor
}

void ParserThread::SkipToOneOfId(const int * idArray, const int num)
{
    RawToken * tk;
    tk = ConsumeToken();
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
        tk = ConsumeToken();
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
    RawToken * tk = PeekToken();
    QUEX_TYPE_TOKEN_ID id = tk->type_id();
    TRACE("Skip statement block Start line(%d) column(%d)\n",tk->line_number(),tk->column_number());

    if (id == TKN_L_BRACE)
    {
        SkipBrace();
    }
    else
    {
        //SkipToId(TKN_SEMICOLON);
        id = tk->type_id();
        while(PeekToken()->type_id()!=TKN_SEMICOLON)
        {
            if(PeekToken()->type_id()==TKN_L_PAREN)
            {
                SkipParentheses();
                continue;
            }

            if (PeekToken()->type_id()==TKN_R_PAREN)
                return;

            if (PeekToken()->type_id() == TKN_L_BRACE)  // {     ---> skip { xxx } block
            {
                SkipBrace();
                continue;
            }
            ConsumeToken();

        }
    }
    TRACE("Skip statement block End line(%d) column(%d)\n",tk->line_number(),tk->column_number());
    m_Context.EndStatement();
}

void ParserThread::SkipParentheses()
{
    //TKN_L_PAREN
    RawToken * tk=ConsumeToken();//remove the first TKN_L_PAREN

    QUEX_TYPE_TOKEN_ID id;
    int level = 1;

    TRACE("Skip Parentheses Start at line(%d)column(%d)\n",tk->line_number(),tk->column_number());
    while(true)
    {
        tk = PeekToken();
        id = tk->type_id();
        if (id == TKN_L_PAREN)
            level++;
        else if (id == TKN_R_PAREN)
            level--;
        ConsumeToken();
        if(level<=0)
            break;
    }
    TRACE("Skip Parentheses End at line(%d)column(%d)\n",tk->line_number(),tk->column_number());

}

void ParserThread::ReadFunctionArguments(ArgumentList &args)
{
    //TKN_L_PAREN
    args.clear();
    ConsumeToken();//remove the first TKN_L_PAREN

    PushContext();
    m_Context.parseType = FunctionParameter;
    DoParse();
    PopContext();

}
void ParserThread::SkipBlock()
{

}

void ParserThread::SkipAngleBraces()
{

}


bool ParserThread::Parse()
{

    TRACE("Start parsing!...................");
    m_Context.inTypedef = false;
    m_Context.parentToken = 0;
    m_Context.parseType = Normal;

    try
    {
        DoParse();
    }
    catch(ParserException& e)
    {
        TRACE("Parse(): End of file");
    }
    m_SymbolTree.Dump();
    return true;
}

void ParserThread::DoParse()
{

    while (true)
    {
        RawToken* tk = PeekToken();

        switch (tk->type_id())
        {
        case TKN_L_BRACE: //{
        {
            SkipBrace();
            break;
        }
        case TKN_R_BRACE: //}
        {
            // the only time we get to find a } is when recursively called by e.g. HandleClass
            // we have to return now...
            TRACE("DoParse(): return from%s%d:%d",tk->get_string().c_str(),tk->line_number(),tk->column_number());
            ConsumeToken();
            return;
        }
        case TKN_R_PAREN: //)
        {
            TRACE("DoParse(): return from%s%d:%d",tk->get_string().c_str(),tk->line_number(),tk->column_number());
            ConsumeToken();
            if(m_Context.parseType==FunctionParameter
               && m_Context.typeQueue.size()>0
               && m_Context.nameQueue.size()>0)
            {
                DoAddToken(tkParameter, m_Context.nameQueue.back().name);
            }
            return;
        }
        case TKN_L_PAREN :       // (
        {
            SkipParentheses();
            break;
        }
        case TKN_FOR:
        case TKN_WHILE:
        case TKN_IF:
        case TKN_SWITCH:
        {
            TRACE("handling for or while block");
            HandleForWhile();
            break;
        }
        case TKN_ELSE:
        {
            ConsumeToken();
            if(PeekToken()->type_id()==TKN_IF)
            {
                ConsumeToken();
                HandleForWhile();
            }
            else
                SkipStatementBlock();
            break;
        }
        case TKN_CLASS:
        case TKN_STRUCT:
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
            ConsumeToken();
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

                if(m_Context.nameQueue.empty())   // AAA(
                {
                    //HandleMacro();
                    if(   m_Context.parentToken!= NULL
                       && m_Context.parentToken->m_SymbolKind == tkClass
                       && m_Context.parentToken->m_Name.get_text() == tk->get_text())
                        HandleFunction();
                    else
                    {
                        SkipParentheses();
                    }

                }
                else                            // AAA BBB(
                {
                    HandleFunction();
                }
                break;
            case  TKN_SEMICOLON:     // A B;
                if(m_Context.nameQueue.empty())   // AAA;
                {
                    SkipStatementBlock();
                }
                else                            // AAA BBB;
                {
                    // the last element of the m_Context.name is the actual variable name
                    cc_string variableName = m_Context.nameQueue.back().name.get_text();

                    // the last element of the m_Context.type is the actual type name
                    cc_string variableType = m_Context.typeQueue.back().name.get_text();

                    TRACE("Variable Find name(%s) type(%s)",variableName.c_str(),variableType.c_str());
                    //Add variable to tokenstree
                    if (m_Options.handleVars)
                        DoAddToken(tkVariable, m_Context.nameQueue.back().name);


                    ConsumeToken();    //consume the semicolon
                    m_Context.EndStatement();
                }
                break;
            case TKN_BITAND://&
                m_Context.typeQualifier.isReference = true;
                ConsumeToken();
                break;
            case TKN_MULT:  //*
                m_Context.typeQualifier.isPointer = true;
                ConsumeToken();
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
            case TKN_DOT:
            case TKN_ARROW:
                ConsumeToken();
                SkipStatementBlock();
                break;

            case TKN_COMMA:
            case TKN_ASSIGN:
                // A B = ....;
                // A B,C,D=2;
                if(m_Context.nameQueue.empty())   // A,
                {
                    TRACE("Something wrong in mode A, ");
                    SkipStatementBlock();
                }
                else                            // A B,
                {
                    cc_string variableName = m_Context.nameQueue.back().name.get_text();
                    cc_string variableType = m_Context.typeQueue.back().name.get_text();
                    TRACE("Variable Find name(%s) type(%s)",variableName.c_str(),variableType.c_str());
                    //Add variable to tokenstree
                    if (m_Options.handleVars)
                        DoAddToken(tkVariable, m_Context.nameQueue.back().name);
                    ConsumeToken();       //consume , or =
                    // tk is updated
                    if(peek->type_id()  == TKN_ASSIGN)
                    {
                        SkipStatementBlock();
                    }
                    else
                        m_Context.nameQueue.clear();  // clear name information, because we need to read another name

                }
                break;
            }
            break;
        }// End of handling of case TKN_IDENTIFIER
        case TKN_SEMICOLON:
        {
            ConsumeToken();
            m_Context.EndStatement();
            break;
        }
        case TKN_PUBLIC:
        {
            ConsumeToken();
            RawToken * peek = PeekToken();
            if(peek->type_id() == TKN_COLON)
            {
                m_Context.accessScope = tsPublic;
                m_Context.EndStatement();
                ConsumeToken();
            }
            break;
        }
        case TKN_PRIVATE:
        {
            ConsumeToken();
            RawToken * peek = PeekToken();
            if(peek->type_id() == TKN_COLON)
            {
                m_Context.accessScope = tsPrivate;
                m_Context.EndStatement();
                ConsumeToken();
            }
            break;
        }
        case TKN_PROTECT:
        {
            ConsumeToken();
            RawToken * peek = PeekToken();
            if(peek->type_id() == TKN_COLON)
            {
                m_Context.accessScope = tsProtected;
                m_Context.EndStatement();
                ConsumeToken();
            }
            break;
        }
        case TKN_USING:
        {
            ParseUsing();
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
            if(!readArgsOK)   //should return true, otherwise, we should
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
        case TKN_NAMESPACE:
        {
           HandleNamespace();
           break;
        }
        case TKN_OPERATOR:
        {
            ConsumeToken(); //eat the operator keyword
            RawToken *op = ConsumeToken();

            // eat the operator pair
            if(op->type_id()==TKN_L_PAREN || op->type_id()==TKN_L_SQUARE)
                ConsumeToken();

            ScopeBlock Operator;
            Operator.name = *op;
            m_Context.nameQueue.push_back(Operator);
            RawToken *peek = PeekToken();
            if(peek->type_id()==TKN_L_PAREN)
            {
                HandleFunction();
            }
            break;
        }
        default:
        {
            //cout<<"Skip unhandled"<<*tk<<endl;
            // As the tk is only a pointer to the RawToken buffer, so it may changed by
            // some function calling on ConsumeToken or PeekToken
            ConsumeToken();
            break;
        }
        }
    }

}


void ParserThread::HandleNamespace()
{
    ConsumeToken();
    RawToken *pk = PeekToken();
    RawToken name;
    if(pk->type_id()==TKN_IDENTIFIER)
    {
        name = *pk;
        ConsumeToken();
    }
    else
    {
        //un-named namespace
    }

    pk = PeekToken();
    if(pk->type_id()==TKN_L_BRACE)
    {
        ConsumeToken();
        Symbol * sym = DoAddToken(tkNamespace,name);
        PushContext();
        m_Context.parentToken = sym;
        DoParse();
        PopContext();
        TRACE("end of namespace\n");
    }

}

void ParserThread::ReadVarNames()
{

}

void ParserThread::ParseUsing()
{
    ConsumeToken();
    RawToken * peek = PeekToken();
    if(peek->type_id() == TKN_NAMESPACE)
    {
        //Handleing using space directive
        ConsumeToken(); //consume "namespace"
        // simply handling mode: using namespace AAA
        peek = PeekToken();
        if (peek->type_id() == TKN_IDENTIFIER)
        {
            ParseFullIdentifer();
            DoAddToken(tkUsingNamespace,*peek);
        }

        else
            SkipStatementBlock();

    }
    else if(peek->type_id() == TKN_IDENTIFIER)
    {
        SkipStatementBlock();
    }
}

void ParserThread::HandleClass(EClassType ct)
{

    // class xxx {   or class {
    // the keyworkd "class" is already comsumed

    //assert(CurrentToken()->type_id()==TKN_CLASS);
    ConsumeToken();


    RawToken * current =  ConsumeToken();      // class name
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
        //parser base clause
        if ( next->type_id() == TKN_COLON )
        {
            //ReadType
            //ConsumeToken(); //remove :id
            ConsumeToken(); //remove  :
            ConsumeToken(); //remove protect/public/private
            //ConsumeToken(); //need to read the id
            ParseFullIdentifer();
        }
        next = PeekToken();
        if ( next->type_id() == TKN_L_BRACE)   // class AAA {, we find the "{" here
        {

            //ParserThreadContext savedContext = m_Context;
            //m_Context.EndStatement();
            //m_Context.parentToken = newToken;
            ConsumeToken();// consume {
            Symbol * sym = DoAddToken(tkClass, *current);
            PushContext();
            m_Context.parentToken = sym;
            DoParse();  // when meet a }, we should return from DoParse()
            PopContext();
            current = ConsumeToken();
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

}

void ParserThread::HandleFunction()
{
    //TRACE("HandleFunction() : %s %s()",m_Context.type.back().name.get_text().c_str(),name.c_str());

    // should always at the ( , so we firstly read the parentheses
    ArgumentList args;
    ReadFunctionArguments(args);

    RawToken * peek = PeekToken();

    if(peek->type_id() == TKN_CONST)
        ConsumeToken();

    // skip the member initialization list
    if(PeekToken()->type_id() == TKN_COLON)
    {
        ConsumeToken();
        while(true)
        {
            if(PeekToken()->type_id()!=TKN_L_BRACE)
                ConsumeToken();
            else
                break;
        }
    }

    if(m_Context.nameQueue.size()==0)
    {
        m_Context.nameQueue = m_Context.typeQueue;
        m_Context.typeQueue.clear();
    }

    peek = PeekToken();
    if (peek->type_id() == TKN_L_BRACE)   // function definition
    {
        TRACE("Function definition");

        //SkipBrace();
        ConsumeToken();
        Symbol * sym = DoAddToken(tkFunction, m_Context.nameQueue.back().name);
        PushContext();
        m_Context.parentToken = sym;
        DoParse();
        PopContext();

    }
    else if (peek->type_id() == TKN_SEMICOLON)
    {
        TRACE("Function declaration");
        DoAddToken(tkFunction, m_Context.nameQueue.back().name);
        //newToken->m_Args = args;
        ConsumeToken();
    }
    m_Context.EndStatement();
}

void ParserThread::ReadEnumList()
{
    RawToken *current;
    RawToken *peek;
    do
    {
        current = ConsumeToken();
        peek    = PeekToken();
        if(current->type_id() == TKN_IDENTIFIER )
        {
            if (peek->type_id() == TKN_ASSIGN)     // a = b,
            {
                //Add variable
                DoAddToken(tkEnumerator, *current);
                ConsumeToken(); //comsume "="
                ConsumeToken(); //comsume the id after "="
            }
            else if (peek->type_id() == TKN_R_BRACE)  //a}
            {
                //Add variable
                DoAddToken(tkEnumerator, *current);
                break;
            }
            else if (peek->type_id() == TKN_IDENTIFIER)      // id p1 p2
            {
                RawToken * peek2 = PeekToken(2);
            }
            else if (peek->type_id() == TKN_COMMA)     // a,....
            {
                //Add variable
                DoAddToken(tkEnumerator, *current);
                ConsumeToken(); //comsume ","
            }
        }
    }
    while(peek->type_id() != TKN_R_BRACE);

}
void ParserThread::HandleEnum()
{
    // enums have the following rough definition:
    // enum [xxx] { type1 name1 [= 1][, [type2 name2 [= 2]]] };

    ConsumeToken();
    RawToken * tk = ConsumeToken();    // the token after "enum"
    //Token * newToken = 0;
    if (tk->type_id() == TKN_IDENTIFIER)           // enum XXX
    {
        RawToken *pk = PeekToken();
        if(pk->type_id() == TKN_L_BRACE )  //enum XXX {
        {
            // Do Add Token of enum
            TRACE("find enum %s at line(%d)",tk->get_text().c_str(),tk->line_number());
            DoAddToken(tkEnum, *tk);
            //newToken->m_ImplLineStart = pk->line_number();
            ConsumeToken();                     //consume {
            PushContext();
            ReadEnumList();
            PopContext();
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
        DoAddToken(tkEnum, *tk);
        ConsumeToken();                    //consume {
        PushContext();
        ReadEnumList();
        PopContext();
    }

    // Token's implement End line information added
    tk = ConsumeToken();
    //we are now after the }

    // if we find a ;, good, end of definition of enum
    // if we find an id, then this is something like enum XXX{...} A,B;
    // A,B were the instant of the enum

    tk = ConsumeToken();
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
    ConsumeToken();

    TRACE("Typedef find");
    //find the token before semicolon
    RawToken * cur;
    RawToken * peek;
    while(true)
    {

        peek = PeekToken();
        if(peek->type_id() == TKN_SEMICOLON)
        {
            ConsumeToken();
            break;
        }
        else
            cur  = ConsumeToken();

    }

    DoAddToken(tkTypedef,*cur);
    //SkipStatementBlock();
}

void ParserThread::ReadClsNames(cc_string& ancestor)
{

}



void ParserThread::PushContext()
{
    m_ContextStack.push(m_Context);
    m_Context.EndStatement();

}
void ParserThread::PopContext()
{
    m_Context = m_ContextStack.top();
    m_ContextStack.pop();
}

bool ParserThread::GetTemplateArgs()
{
    ConsumeToken();

    m_Context.templateArgument.clear();
    int nestLvl = 0;

    while (true)
    {
        RawToken * tk = ConsumeToken();

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
            m_Context.templateArgument.push_back(*tk);
        }
        else
            m_Context.templateArgument.push_back(*tk);

        if (nestLvl <= 0)
        {
            //TRACE("ParserThread::GetTemplateArgs() %s", m_Context.templateArgument.c_str());
            break;
        }

    }
    return true;

}

void ParserThread::SkipBrace()
{
    ConsumeToken();
    int nestLvl = 1;

    while (true)
    {
        RawToken * tk = ConsumeToken();

        if (tk->type_id() == TKN_L_BRACE)
        {
            ++nestLvl;
        }
        else if (tk->type_id() == TKN_R_BRACE) // >
        {
            --nestLvl;
        }

        if (nestLvl <= 0)
        {
            break;
        }

    }

}



bool ParserThread::ParseFullIdentifer()
{
    if(m_Context.typeQueue.size()==0)
        ParseScopeQueue(m_Context.typeQueue);
    else if (m_Context.nameQueue.size()==0)
        ParseScopeQueue(m_Context.nameQueue);
    else { // both type and name has values, so we push the name to type
        m_Context.typeQueue = m_Context.nameQueue;
        // fill the name again
        ParseScopeQueue(m_Context.nameQueue);
    }
    return true;
}
bool ParserThread::ParseScopeQueue(ScopeQueue& scopeQueue)
{
    RawToken * currentToken = ConsumeToken();
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
            ConsumeToken(); //consume ::
            currentToken = ConsumeToken(); // get id
            scope.name = *currentToken;// add the name
            bool parseResult = ParseArgumentList(scope.templateArgumentList);
            if (!parseResult) // this means we have the format A<......;, this is not a template
            {
                m_Context.EndStatement();
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
    RawToken * tk = ConsumeToken();

    TRACE("ParserThread::ParseArgumentList() Enter...");
    int level = 1;
    argumentList.clear();
    argumentList.push_back(*tk); // push <while(true)


    while(true)
    {

        tk = PeekToken();//

        if(tk->type_id() == TKN_LESS)
        {
            ConsumeToken();
            argumentList.push_back(*tk);
            level++;

        }
        else if (tk->type_id() == TKN_GREATER)
        {
            level--;
            ConsumeToken();
            argumentList.push_back(*tk);
            if(level <= 0)
                break;
        }
        else if(tk->type_id()==TKN_SEMICOLON || tk->type_id()==TKN_R_BRACE)
        {
            m_Context.EndStatement();
            return false;
        }
        else
        {
            //std::cout<<*tk<<std::endl;
            argumentList.push_back(*tk);
            ConsumeToken();
        }
    }

    //cout<<"currentToken"<<*(CurrentToken())<<endl;
    TRACE("ParserThread::ParseArgumentList() Leave...");
    return true;
}

void ParserThread::HandleForWhile()
{
    SymbolKind id;
    RawToken * tok = ConsumeToken(); //eat for or while key word
    id = (tok->type_id()==TKN_FOR)? tkFor:tkWhile;
    Symbol * sym = DoAddToken(id,*tok);
    ConsumeToken(); //eat the left parenthesis
    PushContext();  //save the old context
    m_Context.parentToken=sym;
    DoParse();      // do a parse, and should returned on an unbalanced right parenthesis
    PopContext();   // restore the old context

    tok = PeekToken();
    if(tok->type_id()==TKN_L_BRACE)
    {
        ConsumeToken(); //eat {
        PushContext();  //save the old context
        m_Context.parentToken=sym;
        DoParse();      // do a parse, and should returned on an unbalanced right brace
        PopContext();   // restore the old context
    }
    else
        SkipStatementBlock();

}


void ParserThread::TestFunction()
{
    while (true)
    {
        RawToken* tk = ConsumeToken();
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

Symbol *ParserThread::DoAddToken(SymbolKind kind, RawToken & tok)
{
    // add token
    for(int i=0;i<m_ContextStack.size();i++)
        cout<<"   ";

    cout<<GetSymbolKindString(kind)<<" "<<tok.get_text()<<" "<<tok.line_number()<<":"<<tok.column_number()<<endl;

    Symbol * newSymbol = new Symbol(kind,tok);

    m_SymbolTree.insert(newSymbol);

    return newSymbol;
}




int main()
{
    ParserThread parser("test1.cpp");
    parser.Parse();
    return 0;
}

