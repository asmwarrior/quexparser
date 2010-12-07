#ifndef __QUEX_INCLUDE_GUARD__UTF8_HELPER_I
#define __QUEX_INCLUDE_GUARD__UTF8_HELPER_I
/* -*- C++ -*- vim: set syntax=cpp:
 *
 * (C) 2005-2010 Frank-Rene Schaefer                                                */

QUEX_NAMESPACE_MAIN_OPEN

#if ! define(__QUEX_OPTION_WCHAR_T_DISABLED)
QUEX_INLINE QUEX_TYPE_CHARACTER*
/* DrainEnd pointer is not returned, since the increment is always '1' */
QUEX_NAME(utf8_to_wchar_t)(QUEX_TYPE_CHARACTER* input, wchar_t* output)
{
    QUEX_TYPE_CHARACTER*  iterator = input;


    if( (*iterator & 0x80) == 0 ) {
        /* Header: 0xxx.xxxx */
        return (wchar_t)*iterator;
    }
    else if( *iterator < 0xE0 ) { /* ... max: 1101.1111 --> 0xDF, next: 0xE0 */
        /*    110x.xxxx 10yy.yyyy 
         * => 0000.0xxx:xxyy.yyyy  */
        output =    (( ((wchar_t)*(iterator++)) & 0x1F ) << 6) 
                  | (( ((wchar_t)*(iterator++)) & 0x3F )     );
        return iterator;
    }
    else if( *iterator < 0xF0 ) { /* ... max: 1110.1111 --> 0xEF, next: 0xF0 */
        /*    1110.xxxx 10yy.yyyy 10zz.zzzz
         * => xxxx.yyyy:yyzz.zzzz           */
        output =   (( ((wchar_t)*(iterator++)) & 0x0F ) << 12) 
                 | (( ((wchar_t)*(iterator++)) & 0x3F ) << 6 ) 
                 | (( ((wchar_t)*(iterator++)) & 0x3F )      );
        return iterator;
    }
    else {
        /* Unicode standard defines only chars until 0x10ffff, so max(len(utf8char)) == 4.
         *
         * NO CHECK: if( *iterator < 0xF8 ) { ... max: 1111.0111 --> 0xF7, next: 0xF8 
         *
         *    1111.0uuu 10xx.xxxx 10yy.yyyy 10zz.zzzz
         * => 000u.uuxx:xxxx.yyyy:yyzz.zzzz           */
        output =   (( ((wchar_t)*(iterator + 0)) & 0x07 ) << 18) 
                 | (( ((wchar_t)*(iterator + 1)) & 0x3F ) << 12) 
                 | (( ((wchar_t)*(iterator + 2)) & 0x3F ) << 6 ) 
                 | (( ((wchar_t)*(iterator + 3)) & 0x3F )      );
        return iterator;
    }
}
#endif

QUEX_INLINE uint16_t*
QUEX_NAME(utf8_to_utf16_string)(const QUEX_TYPE_CHARACTER* Source, size_t SourceSize,
                                uint16_t* Drain, size_t DrainSize);

#if ! define(__QUEX_OPTION_WCHAR_T_DISABLED)
QUEX_INLINE wchar_t*
QUEX_NAME(utf8_to_wchar_t_string)(const QUEX_TYPE_CHARACTER* Source, size_t SourceSize,
                                  wchar_t* Drain, size_t DrainSize)
{
    const QUEX_TYPE_CHARACTER*  source_iterator, *source_end;
    uint8_t*                    drain_iterator, *drain_end;

    __quex_assert(Source != 0x0);
    __quex_assert(Drain != 0x0);

    drain_iterator = Drain;
    drain_end      = Drain  + DrainSize;
    source_end     = Source + SourceSize;

    for(source_iterator = Source; source_iterator < source_end; ) {
        if( drain_end - drain_iterator < (ptrdiff_t)7 ) break;
        source_iterator = QUEX_NAME($$CODEC$$_to_utf8)(*source_iterator, &drain_iterator);
    }
    return drain_iterator;
}
#endif


#if ! defined(__QUEX_OPTION_PLAIN_C)
QUEX_INLINE std::basic_string<uint16_t>
QUEX_NAME(utf8_to_utf16_string)(const std::basic_string<QUEX_TYPE_CHARACTER>& Source);

QUEX_INLINE std::basic_string<uint32_t>
QUEX_NAME(utf8_to_ucs4_string)(const std::basic_string<QUEX_TYPE_CHARACTER>& Source);
#endif /* __QUEX_OPTION_PLAIN_C */

QUEX_NAMESPACE_MAIN_CLOSE

#endif /* __QUEX_INCLUDE_GUARD__UTF8_HELPER_I */
