
#include <iostream>
#include "tiny_lexer"
using namespace std;


#include "ConstExpression.h"

static quex::tiny_lexer::char_type s_QuexBuffer[4];
quex::tiny_lexer   Quex((QUEX_TYPE_CHARACTER*)s_QuexBuffer,4,(QUEX_TYPE_CHARACTER*)s_QuexBuffer+1);
quex::tiny_lexer::token_type        TokenBuffer[40];

extern int expression_eval(quex::Token *tokenInput);


int main()
{
    char expression[256];
    expression[0]   = 0;
    expression[127] = 0;

    cout << "Entering expression, and Press Enter to run the calculator, enter 'exit' to Exit " << endl;

    while(true) {
        cout << "-> ";
        cin.getline (&expression[1], 255, '\n' );              // Input goes into string

        if (strcmp(&expression[1],"exit") == 0) {
            break;
        }

        int length = strlen(&expression[1]);

        expression[length+1] = 0;

        Quex.reset_buffer((QUEX_TYPE_CHARACTER*)expression,
                          length+2,
                          (QUEX_TYPE_CHARACTER*)expression+length+1);

        (void)Quex.token_p_switch(&TokenBuffer[0]);

        for (int n = 0; ; n++) {
            TokenBuffer[n].text.clear();
            (void)Quex.token_p_switch(&TokenBuffer[n]);
            QUEX_TYPE_TOKEN_ID id = Quex.receive();
            if(id == TKN_TERMINATION )
                break;

        }

        //expression_eval(&TokenBuffer[0]);
        ConstExpression exp;
        exp.expression_eval(&TokenBuffer[0]);

    }



    //printf("%s\n",aaa);
    //printf("the input String length is %d\n",length);








    return 0;
}








