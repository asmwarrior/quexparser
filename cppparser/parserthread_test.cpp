#include <iostream>
#include <fstream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include<iomanip>

#include "parserthread_test.h"


#define PARSERTHREAD_DEBUG_OUTPUT 1


#if PARSERTHREAD_DEBUG_OUTPUT

#define TRACE(format, args...)\
    printf(format , ## args);\
    printf("\n")
#else
#define TRACE(format, ...)
#define DebugLog(format,...)
#endif


ParserThread::ParserThread(const cc_string& bufferOrFilename)
{
        m_Preprocessor.LoadFile(bufferOrFilename);

        try
        {
           m_Preprocessor.RunTest();
        }
        catch(ParserException& e)
        {
            cout<< "end of file" <<endl;
        }

}

ParserThread::~ParserThread()
{
    //dtor
}

void ParserThread::SkipToOneOfId(const int * idArray, const int num)
{
    RawToken * tk;
    tk = GetToken();
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
        tk = GetToken();
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
    RawToken * tk = GetToken();
    QUEX_TYPE_TOKEN_ID id = tk->type_id();
    printf("Skip statement block Start line(%d) column(%d)\n",tk->line_number(),tk->column_number());
    if (id == TKN_L_BRACE)
    {
        //SkipToId(TKN_R_BRACE);
        int level = 1;

        while(level>0)
        {
            tk = GetToken();
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
            tk = GetToken();
            id = tk->type_id();
            if (id == TKN_L_BRACE)  // {     ---> skip { xxx } block
            {
                //SkipToId(TKN_R_BRACE);
                int level = 1;

                while(level>0)
                {
                    tk = GetToken();
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

    RawToken * tk = GetToken();//remove the first TKN_L_PAREN
    QUEX_TYPE_TOKEN_ID id = tk->type_id();
    int level = 1;
    printf("Skip Parentheses Start at line(%d)column(%d)\n",tk->line_number(),tk->column_number());
    while(level>0)
    {
        tk = GetToken();
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

    RawToken * tk = GetToken();//remove the first TKN_L_PAREN

    QUEX_TYPE_TOKEN_ID id = tk->type_id();

    int level = 1;
    printf("Skip Parentheses Start at line(%d)column(%d)\n",tk->line_number(),tk->column_number());
    while(level>0)
    {
        tk = GetToken();
        id = tk->type_id();
        if (id == TKN_L_PAREN)
            level++;
        else if (id == TKN_R_PAREN)
            level--;
        else
            args.push_back(*tk) ;
    }
    printf("Skip Parentheses End at line(%d)column(%d)\n",tk->line_number(),tk->column_number());

}
void ParserThread::SkipBlock()
{

}

void ParserThread::SkipAngleBraces()
{

}


bool ParserThread::Parse()
{

    cout<<"Start parsing!...................\n";
    m_Context.inTypedef = false;
    m_Context.parentToken = 0;

    do
    {
        try
        {
            DoParse();
        }
        catch(ParserException& e)
        {
            cout<< "end of file" <<endl;
        }
    }
    while(false);

    return true;
}

void ParserThread::DoParse()
{

    while (true)
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
                        cout<<"DoAddToken(tkVariable, variableName, line)\n";


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
                        cout<<"DoAddToken(tkVariable, variableName, line)\n";
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
                    cout<<"DoAddToken(tkUsingNamespace,tk->get_text(),tk->line_number())\n";
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
            DoAddToken(tkClass, current->get_text(), current->line_number());
            ParserThreadContext savedContext = m_Context;
            m_Context.EndStatement();
            //m_Context.parentToken = newToken;
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

    RawToken * peek = PeekToken();

    if (peek->type_id() == TKN_L_BRACE)   // function definition
    {
        TRACE("Function definition");
        TokenKind tokenKind = tkFunction ;
        cc_string functionName(name);
        DoAddToken(tokenKind, functionName,peek->line_number());
        //newToken->m_Args = args;
        SkipStatementBlock();
    }
    else if (peek->type_id() == TKN_SEMICOLON)
    {
        TRACE("Function declaration");
        TokenKind tokenKind = tkFunction ;
        cc_string functionName(name);
        DoAddToken(tokenKind, functionName,peek->line_number());
        //newToken->m_Args = args;
        GetToken();
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
                //DoAddToken(tkVariable, current->get_text(), current->line_number());
                GetToken(); //comsume "="
                GetToken(); //comsume the id after "="
            }
            else if (peek->type_id() == TKN_R_BRACE)  //a}
            {
                //Add variable
                //DoAddToken(tkVariable, current->get_text(), current->line_number());
                break;
            }
            else if (peek->type_id() == TKN_IDENTIFIER)      // id p1 p2
            {
                RawToken * peek2 = PeekToken(2);
            }
            else if (peek->type_id() == TKN_COMMA)     // a,....
            {
                //Add variable
                //DoAddToken(tkVariable, current->get_text(), current->line_number());
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
            DoAddToken(tkEnum, tk->get_text(), tk->line_number());
            //newToken->m_ImplLineStart = pk->line_number();
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
        DoAddToken(tkEnum, "UnnamedEnum", tk->line_number());
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
        //newToken->m_ImplLineEnd = tk->line_number();
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
bool ParserThread::ParseScopeQueue(ScopeQueue& scopeQueue)
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
        argumentList.push_back(*tk); // push <
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
            argumentList.push_back(*tk);
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
    while (true)
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

Token *ParserThread::DoAddToken(TokenKind kind,
                      const cc_string& name,
                      int line)
{
    // add token

    cout<<"DoAddToken(): "<<(int)kind<<name<<" "<<line<<endl;
    return NULL;
}




int main()
{
    ParserThread parser("test_vector.cpp");
    parser.Parse();
    return 0;
}

