
#include "tiny_lexer"
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<vector>

// refer to wiki page:
// http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B


enum {ASSOC_NONE=0, ASSOC_LEFT, ASSOC_RIGHT};
enum {OP_BINARY=0, OP_UNARY};

struct Operator
{
    QUEX_TYPE_TOKEN_ID op;
    int prec;
    int assoc;
    int unary;
    int (*eval)(int a1, int a2);
};


class OperatorPrecedenceTable
{
public:
    OperatorPrecedenceTable()
    {
        AddDefaultOperators();
    }

    static int eval_uminus(int a1, int a2);
    static int eval_exp(int a1, int a2);
    static int eval_mul(int a1, int a2);
    static int eval_div(int a1, int a2);
    static int eval_mod(int a1, int a2);
    static int eval_add(int a1, int a2);
    static int eval_sub(int a1, int a2);
    static int eval_not(int a1, int a2);
    static int eval_greater(int a1, int a2);
    static int eval_greater_eq(int a1, int a2);
    static int eval_less(int a1, int a2);
    static int eval_less_eq(int a1, int a2);
    static int eval_eq(int a1, int a2);
    static int eval_not_eq(int a1, int a2);
    static int eval_bitand(int a1, int a2);
    static int eval_bitor(int a1, int a2);
    static int eval_and(int a1, int a2);
    static int eval_or(int a1, int a2);


    void AddDefaultOperators()
    {

        //AddOperator(TKN_L_PAREN,       0, ASSOC_NONE, 0, NULL);
        //AddOperator(TKN_R_PAREN,       0, ASSOC_NONE, 0, NULL);

        AddOperator(TKN_NOT,           11, ASSOC_RIGHT, 1, eval_not);
        //AddOperator(TKN_HASH,          10, ASSOC_RIGHT, 1, eval_uminus);


        AddOperator(TKN_MULT,          9, ASSOC_LEFT, 0, eval_mul);
        AddOperator(TKN_DIV,           9, ASSOC_LEFT, 0, eval_div);
        AddOperator(TKN_MODULO,        9, ASSOC_LEFT, 0, eval_mod);

        AddOperator(TKN_MINUS,         8, ASSOC_LEFT, 1, eval_uminus);

        AddOperator(TKN_PLUS,          7, ASSOC_LEFT, 0, eval_add);
        AddOperator(TKN_MINUS,         7, ASSOC_LEFT, 0, eval_sub);


//    AddOperator(TKN_L_SHIFT,       7, ASSOC_LEFT, 0, NULL);
//    AddOperator(TKN_R_SHIFT,       7, ASSOC_LEFT, 0, NULL);

        AddOperator(TKN_GREATER,       6, ASSOC_LEFT, 0, eval_greater);
        AddOperator(TKN_GREATER_EQ,    6, ASSOC_LEFT, 0, eval_greater_eq);
        AddOperator(TKN_LESS,          6, ASSOC_LEFT, 0, eval_less);
        AddOperator(TKN_LESS_EQ,       6, ASSOC_LEFT, 0, eval_less_eq);

        AddOperator(TKN_EQ,            5, ASSOC_LEFT, 0, eval_eq);
        AddOperator(TKN_NOT_EQ,        5, ASSOC_LEFT, 0, eval_not_eq);

        AddOperator(TKN_BITAND,         4,ASSOC_LEFT, 0, eval_bitand);
        AddOperator(TKN_BITOR,          3,ASSOC_LEFT, 0, eval_bitor);
        AddOperator(TKN_AND,           2, ASSOC_LEFT, 0, eval_and);
        AddOperator(TKN_OR,            1, ASSOC_LEFT, 0, eval_or);
    }

    Operator *GetOperatorInfo(QUEX_TYPE_TOKEN_ID ch, int unary = 0)
    {
        for(unsigned int i=0; i<m_Operators.size(); ++i)
        {
            if(m_Operators[i].op==ch && m_Operators[i].unary==unary )
                return &(m_Operators[i]);
        }
        return NULL;
    }

    void AddOperator( QUEX_TYPE_TOKEN_ID op,
                      int prec,
                      int assoc,
                      int unary,
                      int (*eval)(int a1, int a2))
    {
        Operator newOperator= {op,prec,assoc,unary,eval};
        m_Operators.push_back(newOperator);
    }

    std::vector<Operator> m_Operators;

};





class ConstExpression
{
public:
    ConstExpression():
        m_OperatorStackSize(0),
        m_NumberStackSize(0)
    {

    }
#define MAXOPSTACK 64
#define MAXNUMSTACK 64


    static OperatorPrecedenceTable s_Operators;


    Operator *GetOperator(QUEX_TYPE_TOKEN_ID ch, int a)
    {
        return s_Operators.GetOperatorInfo(ch, a);
    }


    Operator *m_OperatorStack[MAXOPSTACK];
    int m_OperatorStackSize;

    int m_NumberStack[MAXNUMSTACK];
    int m_NumberStackSize;


    bool PushOperatorStack(Operator *op);
    Operator *PopOperatorStack();
    bool PushNumberStack(int num);
    int PopNumberStack();
    void DumpStack();

    quex::Token *m_InputToken;


    bool ShuntOperator(Operator *op);

    int expression_eval(quex::Token *tokenInput);

    int Exp(int pre);


    int Expect(QUEX_TYPE_TOKEN_ID op);
    int P();
    int MakeNode(Operator *op, int v1, int v2);
    quex::Token * Next();
    void Consume();


};
























