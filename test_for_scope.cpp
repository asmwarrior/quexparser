void MyFunction(int paraA, float paramB)
{
    for(int index = 0; Foo* foofoo = Something.getFooByIndex(index); ++index)
    {
       // let's do something with foofoo
       foofoo->DoSomething();
       int i;
       i++;
    }
    
    //type + variable
    for(int a=0;a<10;a++)
    {
       int i;
       i++;
    };
    
    for(NS::MyClass a=0;a<100;a++)
    {
       int i;
       i++;
    };

    // type containing some template info
    for(MyNameSpace::MyTempLateClass<X,Y> a=0;a.DoSomething>b;a++)
        a++;

    // pointer declaration
    for(int *a=0;a<0x4444;a = a+4)
        ;
    for(int **a=0;a<0x4444;a = a+4)
        ;

    // two variables
    for(int *a=0, b=0;...)
		;
}









