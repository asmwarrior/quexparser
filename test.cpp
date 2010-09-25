#define AAA BBB\
CCC DDD EEE\
fff


#define AAA  (

void f();


class MyClass
{
public :

    int a;
	int b;

};



a&&b;
c&&d;

cc_string Tokenizer::PeekToken(bool bGetValue, bool bTemplate)
{
    if(!m_PeekAvailable)
    {
        m_PeekAvailable = true;
        unsigned int undoTokenIndex = m_TokenIndex;
        unsigned int undoLineNumber = m_LineNumber;
        unsigned int undoNestLevel  = m_NestLevel;
        m_PeekToken                 = DoAdvanceGetToken();
        m_PeekTokenIndex            = m_TokenIndex;
        m_PeekLineNumber            = m_LineNumber;
        m_PeekNestLevel             = m_NestLevel;
        m_TokenIndex                = undoTokenIndex;
        m_LineNumber                = undoLineNumber;
        m_NestLevel                 = undoNestLevel;
    }
    return m_PeekToken;
}







