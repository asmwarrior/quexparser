#include "expressionevaluator.h"


#define EXPRESSION_DEBUG 0


#if EXPRESSION_DEBUG

#define TRACE(format, args...)\
    printf(format , ## args);\
    printf("\n")
#else
#define TRACE(format, ...)
#define DebugLog(format,...)
#endif

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
        TRACE("ERROR: Division by zero");
        return 0;
    }
    return a1/a2;
}


int OperatorPrecedenceTable::eval_mod(int a1, int a2)
{
    if(!a2)
    {
        TRACE("ERROR: Division by zero");
        return 0;
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




bool ConstExpression::PushOperatorStack(Operator *op)
{
    if(m_OperatorStackSize>MAXOPSTACK-1)
    {
        TRACE("ERROR: Operator stack overflow");
        return false;
    }
    m_OperatorStack[m_OperatorStackSize++]=op;
    return true;
}

Operator *ConstExpression::PopOperatorStack()
{
    if(!m_OperatorStackSize)
    {
        TRACE("ERROR: Operator stack empty");
        return NULL;
    }
    return m_OperatorStack[--m_OperatorStackSize];
}

bool ConstExpression::PushNumberStack(int num)
{
    if(m_NumberStackSize>MAXNUMSTACK-1)
    {
        TRACE("ERROR: Number stack overflow");
        return false;
    }
    m_NumberStack[m_NumberStackSize++]=num;
    return true;
}

int ConstExpression::PopNumberStack()
{
    if(!m_NumberStackSize)
    {
        TRACE("ERROR: Number stack empty");
        return 0; //FIXME:need to use a more detailed return value
    }
    return m_NumberStack[--m_NumberStackSize];
}

void ConstExpression::DumpStack()
{

    if(!m_NumberStackSize)
    {
        TRACE("dump_stack() num stack: EMPTY!!!");
    }
    else
    {
        // plot the number stack
        //
        TRACE("dump_stack() num stack: ");
        for(int i=0;i<m_NumberStackSize;i++)
            TRACE("%d ",m_NumberStack[i]);
    }


    if(!m_OperatorStackSize)
    {
        TRACE("dump_stack() op stack: EMPTY!!!");
    }
    else
    {
        // plot the operator stack
        TRACE("dump_stack() op stack: ");
        for(int i=0;i<m_OperatorStackSize;i++)
        {
            int id = m_OperatorStack[i]->op;
            quex::tiny_lexer::token_type tk;
            const char * name = tk.map_id_to_name(id);
            TRACE("%s ",name);
        }
    }

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


    for(expr=tokenInput; expr->type_id()!=TKN_PP_FINISH; ++expr)
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
                    TRACE("ERROR: not allowed unary operator (%c)\n", op->op);
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
        else if(expr->type_id() == TKN_DEFINED )
        {
            expr++;
            bool InParentheses = false;
            if (expr->type_id()==TKN_L_PAREN)
            {
                InParentheses = true;
                expr++;
            }
            if(expr->type_id()==TKN_IDENTIFIER)
            {
                //Chech this Id in the symbol table
                bool exist = m_PreProcessor->CheckMacroExist(expr->get_text());
                if (exist==true)
                    PushNumberStack(1);
            }
            else
                PushNumberStack(0);

            if(InParentheses==true)//skip the right parenthesis
                expr++;
            lastOperatorType = NULL;

        }
        else
        {
            TRACE("ERROR: Syntax error, the OP should be either an op or a number");
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
        TRACE("ERROR: Number stack has %d elements after evaluation. Should be 1.", m_NumberStackSize);
        return 0;
    }
    printf("%d\n", m_NumberStack[0]);

    int value = m_NumberStack[0];

    // need to clear all the stack!!

    for (int i = 0; i< MAXOPSTACK; i++)
        m_OperatorStack[i] = NULL;
    m_OperatorStackSize=0;

    for (int i = 0; i< MAXNUMSTACK; i++)
        m_NumberStack[i]=0;
    m_NumberStackSize=0;


    if(value==0)
        return 0;
    else
        return 1;
}




bool ConstExpression::ShuntOperator(Operator *op)
{
    Operator *pop;
    int n1, n2;

    if(!op)
    {
        TRACE("ERROR: op is NULL");
        return false;
    }

    // handling parentheses firstly, since they have no assoc
    if(op->op==TKN_L_PAREN)
    {
        return PushOperatorStack(op);

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
            TRACE("ERROR: Stack error. No matching \'(\'");
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
    if(!PushOperatorStack(op))
        return false;

    return true;
}


OperatorPrecedenceTable ConstExpression::s_Operators;

