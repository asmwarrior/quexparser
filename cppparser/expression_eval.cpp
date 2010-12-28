
#include "tiny_lexer"
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>

#define MAXOPSTACK 64
#define MAXNUMSTACK 64

int eval_uminus(int a1, int a2)
{
    return -a1;
}
int eval_exp(int a1, int a2)
{
    return a2<0 ? 0 : (a2==0?1:a1*eval_exp(a1, a2-1));
}
int eval_mul(int a1, int a2)
{
    return a1*a2;
}
int eval_div(int a1, int a2)
{
    if(!a2) {
        fprintf(stderr, "ERROR: Division by zero\n");
        exit(EXIT_FAILURE);
    }
    return a1/a2;
}
int eval_mod(int a1, int a2)
{
    if(!a2) {
        fprintf(stderr, "ERROR: Division by zero\n");
        exit(EXIT_FAILURE);
    }
    return a1%a2;
}
int eval_add(int a1, int a2)
{
    return a1+a2;
}
int eval_sub(int a1, int a2)
{
    return a1-a2;
}



enum {ASSOC_NONE=0, ASSOC_LEFT, ASSOC_RIGHT};

struct op_s {
    QUEX_TYPE_TOKEN_ID op;
    int prec;
    int assoc;
    int unary;
    int (*eval)(int a1, int a2);
} ops[]= {
    {TKN_HASH, 10, ASSOC_RIGHT, 1, eval_uminus},
    {TKN_XOR, 9, ASSOC_RIGHT, 0, eval_exp},
    {TKN_MULT, 8, ASSOC_LEFT, 0, eval_mul},
    {TKN_DIV, 8, ASSOC_LEFT, 0, eval_div},
    {TKN_MODULO, 8, ASSOC_LEFT, 0, eval_mod},
    {TKN_PLUS, 5, ASSOC_LEFT, 0, eval_add},
    {TKN_MINUS, 5, ASSOC_LEFT, 0, eval_sub},
    {TKN_L_PAREN, 0, ASSOC_NONE, 0, NULL},
    {TKN_R_PAREN, 0, ASSOC_NONE, 0, NULL}
};





struct op_s *getop(QUEX_TYPE_TOKEN_ID ch) {
    int i;
    for(i=0; i<sizeof ops/sizeof ops[0]; ++i) {
        if(ops[i].op==ch) return ops+i;
    }
    return NULL;
}


struct op_s *opstack[MAXOPSTACK];
int nopstack=0;

int numstack[MAXNUMSTACK];
int nnumstack=0;




void push_opstack(struct op_s *op)
{
    if(nopstack>MAXOPSTACK-1) {
        fprintf(stderr, "ERROR: Operator stack overflow\n");
        exit(EXIT_FAILURE);
    }
    opstack[nopstack++]=op;
}

struct op_s *pop_opstack() {
    if(!nopstack) {
        fprintf(stderr, "ERROR: Operator stack empty\n");
        exit(EXIT_FAILURE);
    }
    return opstack[--nopstack];
}

void push_numstack(int num)
{
    if(nnumstack>MAXNUMSTACK-1) {
        fprintf(stderr, "ERROR: Number stack overflow\n");
        exit(EXIT_FAILURE);
    }
    numstack[nnumstack++]=num;
}

int pop_numstack()
{
    if(!nnumstack) {
        fprintf(stderr, "ERROR: Number stack empty\n");
        exit(EXIT_FAILURE);
    }
    return numstack[--nnumstack];
}



void shunt_op(struct op_s *op)
{
    struct op_s *pop;
    int n1, n2;

    if(op->op==TKN_L_PAREN) {
        push_opstack(op);
        return;

    } else if(op->op==TKN_R_PAREN) {
        while(nopstack>0 && opstack[nopstack-1]->op!=TKN_L_PAREN) {
            pop=pop_opstack();
            n1=pop_numstack();

            if(pop->unary) push_numstack(pop->eval(n1, 0));
            else {
                n2=pop_numstack();
                push_numstack(pop->eval(n2, n1));
            }
        }

        if(!(pop=pop_opstack()) || pop->op!=TKN_L_PAREN) {
            fprintf(stderr, "ERROR: Stack error. No matching \'(\'\n");
            exit(EXIT_FAILURE);
        }
        return;
    }

    if(op->assoc==ASSOC_RIGHT) {
        while(nopstack && op->prec<opstack[nopstack-1]->prec) {
            pop=pop_opstack();
            n1=pop_numstack();
            if(pop->unary) push_numstack(pop->eval(n1, 0));
            else {
                n2=pop_numstack();
                push_numstack(pop->eval(n2, n1));
            }
        }
    } else {
        while(nopstack && op->prec<=opstack[nopstack-1]->prec) {
            pop=pop_opstack();
            n1=pop_numstack();
            if(pop->unary) push_numstack(pop->eval(n1, 0));
            else {
                n2=pop_numstack();
                push_numstack(pop->eval(n2, n1));
            }
        }
    }
    push_opstack(op);
}


int expression_eval(quex::Token *tokenInput)
{
    quex::Token *expr;
    quex::Token *tstart=NULL;
    struct op_s startop= {TKN_ASM, 0, ASSOC_NONE, 0, NULL};	/* Dummy operator to mark start */
    struct op_s *op=NULL;
    int n1, n2;
    struct op_s *lastop=&startop;


    for(expr=tokenInput; expr->type_id()!=TKN_TERMINATION; ++expr) {

        if(!tstart) {

            if( op=getop(expr->type_id()) ) {

                if(lastop && (lastop==&startop || lastop->op!=TKN_R_PAREN)) {
                    if(op->op==TKN_MINUS)
                        op=getop(TKN_HASH);
                    else if(op->op!=TKN_L_PAREN) {
                        fprintf(stderr, "ERROR: Illegal use of binary operator (%c)\n", op->op);
                        exit(EXIT_FAILURE);
                    }
                }
                shunt_op(op);
                lastop=op;
            }
            else if( expr->type_id() == TKN_NUMBER)
                tstart=expr;
            else if(expr->type_id() != TKN_NUMBER) {
                fprintf(stderr, "ERROR: Syntax error\n");
                return EXIT_FAILURE;
            }
        }
        else {
            if(expr->type_id() == TKN_NUMBER) {
                push_numstack(atoi(tstart->get_text().c_str()));
                tstart=NULL;
                lastop=NULL;
            }
            else if( op=getop(expr->type_id()) ) {
                push_numstack(atoi(tstart->get_text().c_str()));
                tstart=NULL;
                shunt_op(op);
                lastop=op;
            }
            else if(expr->type_id() != TKN_NUMBER) {
                fprintf(stderr, "ERROR: Syntax error\n");
                return EXIT_FAILURE;
            }
        }
    }
    if(tstart) push_numstack(atoi(tstart->get_text().c_str()));


    while(nopstack) {
        op=pop_opstack();
        n1=pop_numstack();
        if(op->unary)
            push_numstack(op->eval(n1, 0));
        else {
            n2=pop_numstack();
            push_numstack(op->eval(n2, n1));
        }
    }

    if(nnumstack!=1) {
        fprintf(stderr, "ERROR: Number stack has %d elements after evaluation. Should be 1.\n", nnumstack);
        return EXIT_FAILURE;
    }
    printf("%d\n", numstack[0]);

    return EXIT_SUCCESS;
}
