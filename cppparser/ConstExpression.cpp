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
        return 0;
    }
    return a1/a2;
}


int OperatorPrecedenceTable::eval_mod(int a1, int a2)
{
    if(!a2)
    {
        fprintf(stderr, "ERROR: Division by zero\n");
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
        fprintf(stderr, "ERROR: Operator stack overflow\n");
        return false;
    }
    m_OperatorStack[m_OperatorStackSize++]=op;
    return true;
}

Operator *ConstExpression::PopOperatorStack()
{
    if(!m_OperatorStackSize)
    {
        fprintf(stderr, "ERROR: Operator stack empty\n");
        return NULL;
    }
    return m_OperatorStack[--m_OperatorStackSize];
}

bool ConstExpression::PushNumberStack(int num)
{
    if(m_NumberStackSize>MAXNUMSTACK-1)
    {
        fprintf(stderr, "ERROR: Number stack overflow\n");
        return false;
    }
    m_NumberStack[m_NumberStackSize++]=num;
    return true;
}

int ConstExpression::PopNumberStack()
{
    if(!m_NumberStackSize)
    {
        fprintf(stderr, "ERROR: Number stack empty\n");
        return 0; //FIXME:need to use a more detailed return value
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





/**
Eparser is
   var t : Tree
   t := Exp( 0 )
   expect( end )
   return t

Exp( p ) is
    var t : Tree
    t := P
    while next is a binary operator and prec(binary(next)) >= p
       const op := binary(next)
       consume
       const q := case associativity(op)
                  of Right: prec( op )
                     Left:  1+prec( op )
       const t1 := Exp( q )
       t := mkNode( op, t, t1)
    return t

P is
    if next is a unary operator
         const op := unary(next)
         consume
         q := prec( op )
         const t := Exp( q )
         return mkNode( op, t )
    else if next = "("
         consume
         const t := Exp( 0 )
         expect ")"
         return t
    else if next is a v
         const t := mkLeaf( next )
         consume
         return t
    else
         error
**/



int ConstExpression::Exp(int pre)
{
    int t = P();

    Operator *op;
    while( Next()->type_id()>0
          && (op=GetOperator(Next()->type_id(),0))
          && op->prec >= pre)
    {
        Consume();
        int q;
        if(op->assoc == ASSOC_LEFT)
            q = op->prec+1;
        else if (op->assoc == ASSOC_RIGHT)
            q = op->prec;
        int t1 = Exp(q);
        t = MakeNode( op, t, t1);
    }
    return t;

}

/**
expect( tok ) is
   if next = tok
       consume
   else
       error
*/
int ConstExpression::Expect(QUEX_TYPE_TOKEN_ID op)
{
    if (Next()->type_id() == op)
        Consume();
    else
        exit(0);

}
int ConstExpression::P()
{
    Operator *op;
    if(op=GetOperator(Next()->type_id(),1)) // unary
    {
        Consume();
        int q = op->prec;
        int t = Exp(q);
        return MakeNode(op,t,0);
    }
    else if(Next()->type_id()==TKN_L_PAREN)
    {
        Consume();
        int t = Exp(0);
        Expect(TKN_R_PAREN);
        return t;
    }
    else if(Next()->type_id()==TKN_NUMBER)
    {
        int t = atoi(Next()->get_text().c_str());
        Consume();
        return t;
    }
    else
        exit(0);

}
int ConstExpression::MakeNode(Operator *op, int v1, int v2)
{
    return op->eval(v1,v2);
}

quex::Token *ConstExpression::Next()
{
        return m_InputToken;
}

void ConstExpression::Consume()
{
    if(m_InputToken->type_id()!=TKN_TERMINATION)
        m_InputToken++;
}




int ConstExpression::expression_eval(quex::Token *tokenInput)
{
    m_InputToken = tokenInput;
    int t = Exp(0);
    Expect(TKN_TERMINATION);
    std::cout<<t<<std::endl;
    return EXIT_SUCCESS;
}


OperatorPrecedenceTable ConstExpression::s_Operators;

