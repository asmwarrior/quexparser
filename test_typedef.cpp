

    // typedefs are handled as tkClass and we put the typedef'd type as the
    // class's ancestor. This way, it will work through inheritance.
    // Function pointers are a different beast and are handled differently.

    // this is going to be tough :(
    // let's see some examples:
    //
    // relatively easy:
	
	
    typedef unsigned int uint32;
    typedef std::map<String, StringVector> AnimableDictionaryMap;
    typedef class { int a; int b;} type;
	
    //
    // harder:
    typedef void dMessageFunction (int errnum, const char *msg, va_list ap);
    //
    // even harder:
    typedef void (*dMessageFunction)(int errnum, const char *msg, va_list ap);
