#include "ConstExpression.h"


int OperatorPrecedenceTable::eval_uminus(int a1, int a2)
{
    return -a1;
}
int OperatorPrecedenceTable::eval_exp(int a1, int a2)
{
    return a2<0 ? 0 : (a2==0?1:a1*eval_exp(a1, a2-1));
}
int OperatorPrecedenceTable::eval_mul(int a1, int a2)
{
    return a1*a2;
}
int OperatorPrecedenceTable::eval_div(int a1, int a2)
{
    if(!a2)
    {
        fprintf(stderr, "ERROR: Division by zero\n");
        exit(EXIT_FAILURE);
    }
    return a1/a2;
}


int OperatorPrecedenceTable::eval_mod(int a1, int a2)
{
    if(!a2)
    {
        fprintf(stderr, "ERROR: Division by zero\n");
        exit(EXIT_FAILURE);
    }
    return a1%a2;
}
int OperatorPrecedenceTable::eval_add(int a1, int a2)
{
    return a1+a2;
}

int OperatorPrecedenceTable::eval_sub(int a1, int a2)
{
    return a1-a2;
}

int OperatorPrecedenceTable::eval_not(int a1, int a2)
{
    if(a1>0)
        return 0;
    else
        return 1;
}

int OperatorPrecedenceTable::eval_greater(int a1, int a2)
{
    if(a1-a2>0)
        return 1;
    else
        return 0;
}

int OperatorPrecedenceTable::eval_greater_eq(int a1, int a2)
{
    if(a1-a2>=0)
        return 1;
    else
        return 0;
}

int OperatorPrecedenceTable::eval_less(int a1, int a2)
{
    if(a1-a2<0)
        return 1;
    else
        return 0;
}

int OperatorPrecedenceTable::eval_less_eq(int a1, int a2)
{
    if(a1-a2<=0)
        return 1;
    else
        return 0;
}

int OperatorPrecedenceTable::eval_eq(int a1, int a2)
{
    if(a1==a2)
        return 1;
    else
        return 0;
}


int OperatorPrecedenceTable::eval_not_eq(int a1, int a2)
{
    if(a1==a2)
        return 1;
    else
        return 0;
}



int OperatorPrecedenceTable::eval_bitand(int a1, int a2)
{
    return a1&a2;
}

int OperatorPrecedenceTable::eval_bitor(int a1, int a2)
{
    return a1|a2;
}

int OperatorPrecedenceTable::eval_and(int a1, int a2)
{
    if(a1==0 || a2==0)
        return 0;
    else
        return 1;
}

int OperatorPrecedenceTable::eval_or(int a1, int a2)
{
    if(a1>0 || a2>0)
        return 1;
    else
        return 0;
}




void ConstExpression::push_opstack(struct op_s *op)
{
    if(nopstack>MAXOPSTACK-1)
    {
        fprintf(stderr, "ERROR: Operator stack overflow\n");
        exit(EXIT_FAILURE);
    }
    opstack[nopstack++]=op;
}

op_s *ConstExpression::pop_opstack()
{
    if(!nopstack)
    {
        fprintf(stderr, "ERROR: Operator stack empty\n");
        exit(EXIT_FAILURE);
    }
    return opstack[--nopstack];
}

void ConstExpression::push_numstack(int num)
{
    if(nnumstack>MAXNUMSTACK-1)
    {
        fprintf(stderr, "ERROR: Number stack overflow\n");
        exit(EXIT_FAILURE);
    }
    numstack[nnumstack++]=num;
}

int ConstExpression::pop_numstack()
{
    if(!nnumstack)
    {
        fprintf(stderr, "ERROR: Number stack empty\n");
        exit(EXIT_FAILURE);
    }
    return numstack[--nnumstack];
}

void ConstExpression::dump_stack()
{

    if(!nnumstack)
    {
        fprintf(stderr, "ERROR: Number stack empty\n");
        return;
    }
    // plot the number stack
    //
    fprintf(stdout, "num stack: ");
    for(int i=0;i<nnumstack;i++)
        fprintf(stdout, "%d ",numstack[i]);

    fprintf(stdout, "\n");
    if(!nopstack)
    {
        fprintf(stderr, "ERROR: Operator stack empty\n");
        return;
    }
    // plot the operator stack
    fprintf(stdout, "op stack: ");
    for(int i=0;i<nopstack;i++)
    {
        int id = opstack[i]->op;
        quex::tiny_lexer::token_type tk;
        const char * name = tk.map_id_to_name(id);
        fprintf(stdout, "%s ",name);
    }

    fprintf(stdout, "\n");

}


int ConstExpression::expression_eval(quex::Token *tokenInput)
{
    // a pointer walk through the expression input from left to right
    quex::Token *expr;

    quex::Token *tstart=NULL;
    struct op_s startop= {TKN_ASM, 0, ASSOC_NONE, 0, NULL};	/* Dummy operator to mark start */
    struct op_s *op=NULL;
    int n1, n2;
    struct op_s *lastop=&startop;


    for(expr=tokenInput; expr->type_id()!=TKN_TERMINATION; ++expr)
    {

        if(!tstart)
        {

            // check if it is a valid operator, otherwise, it should be a number
            if( op=getop(expr->type_id()) )
            {

                // it is an opeartor
                if(lastop && (lastop==&startop || lastop->op!=TKN_R_PAREN))
                {
                    //unary operator?
                    if(op->op==TKN_MINUS) // this minus is a negtive unaray operator, because it is after a operator or (
                        op=getop(TKN_HASH); // unary -
                    else if (op->op==TKN_NOT)
                        ;                 // unary !
                    else if (op->op==TKN_PLUS)
                        continue;         // just skip the unary +
                    else if(op->op!=TKN_L_PAREN)
                    {
                        fprintf(stderr, "ERROR: Illegal use of binary operator (%c)\n", op->op);
                        exit(EXIT_FAILURE);
                    }
                }
                shunt_op(op);
                lastop=op;
            }
            else if( expr->type_id() == TKN_NUMBER)
                tstart=expr;
            else if(expr->type_id() != TKN_NUMBER)
            {
                fprintf(stderr, "ERROR: Syntax error\n");
                return EXIT_FAILURE;
            }
        }
        else
        {
            if(expr->type_id() == TKN_NUMBER)
            {
                push_numstack(atoi(tstart->get_text().c_str()));
                tstart=NULL;
                lastop=NULL;
            }
            else if( op=getop(expr->type_id()) )
            {
                push_numstack(atoi(tstart->get_text().c_str()));
                tstart=NULL;
                shunt_op(op);
                lastop=op;
            }
            else if(expr->type_id() != TKN_NUMBER)
            {
                fprintf(stderr, "ERROR: Syntax error\n");
                return EXIT_FAILURE;
            }
        }

        //debug only
        dump_stack();
    }

    if(tstart)
        push_numstack(atoi(tstart->get_text().c_str()));


    while(nopstack)
    {
        op=pop_opstack();
        n1=pop_numstack();
        if(op->unary)
            push_numstack(op->eval(n1, 0));
        else
        {
            n2=pop_numstack();
            push_numstack(op->eval(n2, n1));
        }
    }

    if(nnumstack!=1)
    {
        fprintf(stderr, "ERROR: Number stack has %d elements after evaluation. Should be 1.\n", nnumstack);
        return EXIT_FAILURE;
    }
    printf("%d\n", numstack[0]);

    // need to clear all the stack!!

    for (int i = 0; i< MAXOPSTACK; i++)
        opstack[i] = NULL;
    nopstack=0;

    for (int i = 0; i< MAXNUMSTACK; i++)
        numstack[i]=0;
    nnumstack=0;


    return EXIT_SUCCESS;
}




void ConstExpression::shunt_op(struct op_s *op)
{
    struct op_s *pop;
    int n1, n2;

    if(!op)
    {
        fprintf(stderr, "ERROR: op is NULL\n");
        exit(EXIT_FAILURE);
    }

    // handling parenthese firstly, since they have no assoc
    if(op->op==TKN_L_PAREN)
    {
        push_opstack(op);
        return;

    }
    else if(op->op==TKN_R_PAREN)
    {
        while(nopstack>0 && opstack[nopstack-1]->op!=TKN_L_PAREN)
        {
            pop=pop_opstack();
            n1=pop_numstack();

            if(pop->unary)
                push_numstack(pop->eval(n1, 0));
            else
            {
                n2=pop_numstack();
                push_numstack(pop->eval(n2, n1));
            }
        }

        if(!(pop=pop_opstack()) || pop->op!=TKN_L_PAREN)
        {
            fprintf(stderr, "ERROR: Stack error. No matching \'(\'\n");
            exit(EXIT_FAILURE);
        }
        return;
    }

    if(op->assoc==ASSOC_RIGHT)
    {

        while (nopstack && op->prec < opstack[nopstack-1]->prec )
        {

            //when going here, means the meeted op has lower precedence as on the opstack
            //so, we just pop up

            //pay attention: equal is not allowed, this means for a right_assoc op, we just push!
            //so once the op stack was poped, the later pushed op (as we move from left to right) will
            //be caculated firstly, this confirmed the right_assoc rule.

            pop=pop_opstack();// this will internally change the nopstack
            n1=pop_numstack();// this will internally change the nnumstack

            if(pop->unary)
                push_numstack(pop->eval(n1, 0));
            else
            {
                n2=pop_numstack();
                push_numstack(pop->eval(n2, n1));
            }
        }
    }
    else     //left assoc
    {
        while (nopstack && op->prec <= opstack[nopstack-1]->prec )
        {

            // if it is a left_assoc op, then equal precedence is allowed, once meat this, the op in the
            // stack should be caculated firstly (thus, the left op was caculated firstly).

            pop=pop_opstack();
            n1=pop_numstack();
            if(pop->unary)
                push_numstack(pop->eval(n1, 0));
            else
            {
                n2=pop_numstack();
                push_numstack(pop->eval(n2, n1));
            }
        }
    }
    push_opstack(op);
}


OperatorPrecedenceTable ConstExpression::s_Operators;

