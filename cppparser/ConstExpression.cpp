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




void ConstExpression::PushOperatorStack(Operator *op)
{
    if(m_OperatorStackSize>MAXOPSTACK-1)
    {
        fprintf(stderr, "ERROR: Operator stack overflow\n");
        exit(EXIT_FAILURE);
    }
    m_OperatorStack[m_OperatorStackSize++]=op;
}

Operator *ConstExpression::PopOperatorStack()
{
    if(!m_OperatorStackSize)
    {
        fprintf(stderr, "ERROR: Operator stack empty\n");
        exit(EXIT_FAILURE);
    }
    return m_OperatorStack[--m_OperatorStackSize];
}

void ConstExpression::PushNumberStack(int num)
{
    if(m_NumberStackSize>MAXNUMSTACK-1)
    {
        fprintf(stderr, "ERROR: Number stack overflow\n");
        exit(EXIT_FAILURE);
    }
    m_NumberStack[m_NumberStackSize++]=num;
}

int ConstExpression::PopNumberStack()
{
    if(!m_NumberStackSize)
    {
        fprintf(stderr, "ERROR: Number stack empty\n");
        exit(EXIT_FAILURE);
    }
    return m_NumberStack[--m_NumberStackSize];
}

void ConstExpression::DumpStack()
{

    if(!m_NumberStackSize)
    {
        fprintf(stderr, "dump_stack() num stack: EMPTY!!!");
    }
    else
    {
        // plot the number stack
        //
        fprintf(stdout, "dump_stack() num stack: ");
        for(int i=0;i<m_NumberStackSize;i++)
            fprintf(stdout, "%d ",m_NumberStack[i]);
    }
    fprintf(stdout, "\n");


    if(!m_OperatorStackSize)
    {
        fprintf(stderr, "dump_stack() op stack: EMPTY!!!");
    }
    else
    {
        // plot the operator stack
        fprintf(stdout, "dump_stack() op stack: ");
        for(int i=0;i<m_OperatorStackSize;i++)
        {
            int id = m_OperatorStack[i]->op;
            quex::tiny_lexer::token_type tk;
            const char * name = tk.map_id_to_name(id);
            fprintf(stdout, "%s ",name);
        }
    }
    fprintf(stdout, "\n");

}


int ConstExpression::expression_eval(quex::Token *tokenInput)
{
    // a pointer walk through the expression input from left to right
    quex::Token *expr;

    /* Dummy operator to mark start */
    Operator nilOperator = {TKN_ASM, 0, ASSOC_NONE, 0, NULL};
    Operator    * lastOperatorType =&nilOperator ;


    Operator *op=NULL;
    int n1, n2;


    for(expr=tokenInput; expr->type_id()!=TKN_TERMINATION; ++expr)
    {

        op = GetOperator(expr->type_id());

        // check if it is a valid operator, otherwise, it should be a number

        if( op )//this is an operator or left/right parenthesis
        {
            // check if it was a unary operator
            if( lastOperatorType
               && (lastOperatorType == &nilOperator
                  || lastOperatorType->op == TKN_L_PAREN
                  || lastOperatorType->op == TKN_PLUS
                  || lastOperatorType->op == TKN_MINUS))
            {
                // currently, only three type of unary operators are used
                // they are: unary - ; unary + ; unary !
                // this minus is a negative unary operator
                if(op->op==TKN_MINUS)
                    op=GetOperator(TKN_HASH); // unary -
                else if (op->op==TKN_NOT)
                    ;                         // unary !, handle this normally
                else if (op->op==TKN_PLUS)
                    continue;                 // just skip the unary +
                else if (op->op==TKN_L_PAREN)
                    ;                         //handle this normally
                else
                {
                    fprintf(stderr, "ERROR: not allowed unary operator (%c)\n", op->op);
                    //exit(EXIT_FAILURE);
                    return 0;
                }
            }
            //handling this operator
            if(!ShuntOperator(op))
                return 0;
            //save the current op

            lastOperatorType = op;
        }
        else if( expr->type_id() == TKN_NUMBER ) // if it is not an operator, it should be a number
        {
            PushNumberStack(atoi(expr->get_text().c_str()));
            lastOperatorType = NULL;
        }
        else
        {
            fprintf(stderr, "ERROR: Syntax error, the OP should be either an op or a number\n");
            return 0;
        }
        //debug only
        DumpStack();
    }


    while(m_OperatorStackSize)
    {
        op=PopOperatorStack();
        n1=PopNumberStack();
        if(op->unary)
            PushNumberStack(op->eval(n1, 0));
        else
        {
            n2=PopNumberStack();
            PushNumberStack(op->eval(n2, n1));
        }
    }

    if(m_NumberStackSize!=1)
    {
        fprintf(stderr, "ERROR: Number stack has %d elements after evaluation. Should be 1.\n", m_NumberStackSize);
        return EXIT_FAILURE;
    }
    printf("%d\n", m_NumberStack[0]);

    // need to clear all the stack!!

    for (int i = 0; i< MAXOPSTACK; i++)
        m_OperatorStack[i] = NULL;
    m_OperatorStackSize=0;

    for (int i = 0; i< MAXNUMSTACK; i++)
        m_NumberStack[i]=0;
    m_NumberStackSize=0;


    return EXIT_SUCCESS;
}




bool ConstExpression::ShuntOperator(Operator *op)
{
    Operator *pop;
    int n1, n2;

    if(!op)
    {
        fprintf(stderr, "ERROR: op is NULL\n");
        //exit(EXIT_FAILURE);
        return false;
    }

    // handling parentheses firstly, since they have no assoc
    if(op->op==TKN_L_PAREN)
    {
        PushOperatorStack(op);
        return true;

    }
    else if(op->op==TKN_R_PAREN)
    {
        while(m_OperatorStackSize>0 && m_OperatorStack[m_OperatorStackSize-1]->op!=TKN_L_PAREN)
        {
            pop=PopOperatorStack();
            n1=PopNumberStack();

            if(pop->unary)
                PushNumberStack(pop->eval(n1, 0));
            else
            {
                n2=PopNumberStack();
                PushNumberStack(pop->eval(n2, n1));
            }
        }

        if(!(pop=PopOperatorStack()) || pop->op!=TKN_L_PAREN)
        {
            fprintf(stderr, "ERROR: Stack error. No matching \'(\'\n");
            //exit(EXIT_FAILURE);
            return false;
        }
        return true;
    }

    if(op->assoc==ASSOC_RIGHT)
    {

        while (m_OperatorStackSize && op->prec < m_OperatorStack[m_OperatorStackSize-1]->prec )
        {

            //when going here, means the meeted op has lower precedence as on the OperatorStack
            //so, we just pop up

            //pay attention: equal is not allowed, this means for a right_assoc op, we just push!
            //so once the op stack was poped, the later pushed op (as we move from left to right) will
            //be calculated firstly, this confirmed the right_assoc rule.

            pop=PopOperatorStack();// this will internally change the nopstack
            n1=PopNumberStack();// this will internally change the nnumstack

            if(pop->unary)
                PushNumberStack(pop->eval(n1, 0));
            else
            {
                n2=PopNumberStack();
                PushNumberStack(pop->eval(n2, n1));
            }
        }
    }
    else     //left assoc
    {
        while (m_OperatorStackSize && op->prec <= m_OperatorStack[m_OperatorStackSize-1]->prec )
        {

            // if it is a left_assoc op, then equal precedence is allowed, once meat this, the op in the
            // stack should be calculated firstly (thus, the left op was calculated firstly).

            pop=PopOperatorStack();
            n1=PopNumberStack();
            if(pop->unary)
                PushNumberStack(pop->eval(n1, 0));
            else
            {
                n2=PopNumberStack();
                PushNumberStack(pop->eval(n2, n1));
            }
        }
    }
    PushOperatorStack(op);

    return true;
}


OperatorPrecedenceTable ConstExpression::s_Operators;

