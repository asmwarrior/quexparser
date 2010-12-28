

#include "tiny_lexer"

static QUEX_TYPE_CHARACTER s_QuexBuffer[4];
quex::tiny_lexer   Quex((QUEX_TYPE_CHARACTER*)s_QuexBuffer,4,(QUEX_TYPE_CHARACTER*)s_QuexBuffer+1);
quex::Token        TokenBuffer[40];

extern int expression_eval(quex::Token *tokenInput);
int main()
{
    char aaa[] = "  (1 + 3) * 5 -80/20  \0";
    int length = strlen(aaa);
    printf("%s\n",aaa);
    printf("the input String length is %d\n",length);

    Quex.reset_buffer((QUEX_TYPE_CHARACTER*)aaa,
                                       strlen(aaa),
                                       (QUEX_TYPE_CHARACTER*)aaa+strlen(aaa)-1);

    (void)Quex.token_p_switch(&TokenBuffer[0]);



    for (int n = 0; ;n++)
    {
        TokenBuffer[n].text.clear();
        (void)Quex.token_p_switch(&TokenBuffer[n]);
        QUEX_TYPE_TOKEN_ID id = Quex.receive();
        if(id == TKN_TERMINATION )
            break;

    }

    expression_eval(&TokenBuffer[0]);


    return 0;
}








