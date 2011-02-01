
#include "tiny_lexer"
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>

class ConstExpression
{
public:
    ConstExpression():
       nopstack(0),
        nnumstack(0)
    {

    }
#define MAXOPSTACK 64
#define MAXNUMSTACK 64

    enum {ASSOC_NONE=0, ASSOC_LEFT, ASSOC_RIGHT};

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



// refer to wiki page:
// http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B

    struct op_s
    {
        QUEX_TYPE_TOKEN_ID op;
        int prec;
        int assoc;
        int unary;
        int (*eval)(int a1, int a2);
    };

    static struct op_s ops[19];


    struct op_s *getop(QUEX_TYPE_TOKEN_ID ch)
    {
        int i;
        for(i=0; i<sizeof ops/sizeof ops[0]; ++i)
        {
            if(ops[i].op==ch) return ops+i;
        }
        return NULL;
    }


    struct op_s *opstack[MAXOPSTACK];
    int nopstack;

    int numstack[MAXNUMSTACK];
    int nnumstack;


    void push_opstack(struct op_s *op);
    struct op_s *pop_opstack();
    void push_numstack(int num);
    int pop_numstack();


    void shunt_op(struct op_s *op);

    int expression_eval(quex::Token *tokenInput);

};


























