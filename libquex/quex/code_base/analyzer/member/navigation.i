/* -*- C++ -*- vim: set syntax=cpp:
 * (C) 2005-2010 Frank-Rene Schaefer
 * ABSOLUTELY NO WARRANTY              */
#ifndef __QUEX_INCLUDE_GUARD__ANALYZER__MEMBER__NAVIGATION_I
#define __QUEX_INCLUDE_GUARD__ANALYZER__MEMBER__NAVIGATION_I

QUEX_NAMESPACE_MAIN_OPEN

QUEX_INLINE size_t  
QUEX_NAME(tell)(QUEX_TYPE_ANALYZER* me)
{
    /* No 'undo terminating zero' -- we do not change the lexeme pointer here. */
    return QUEX_NAME(Buffer_tell)(&me->buffer);
}

QUEX_INLINE void    
QUEX_NAME(seek)(QUEX_TYPE_ANALYZER* me, const size_t CharacterIndex)
{
    QUEX_NAME(Buffer_undo_terminating_zero_for_lexeme)(&me->buffer);
    QUEX_NAME(Buffer_seek)(&me->buffer, CharacterIndex);
}

QUEX_INLINE void    
QUEX_NAME(seek_forward)(QUEX_TYPE_ANALYZER* me, const size_t CharacterN)
{
    QUEX_NAME(Buffer_undo_terminating_zero_for_lexeme)(&me->buffer);
    QUEX_NAME(Buffer_move_forward)(&me->buffer, CharacterN);
}

QUEX_INLINE void    
QUEX_NAME(seek_backward)(QUEX_TYPE_ANALYZER* me, const size_t CharacterN)
{
    QUEX_NAME(Buffer_undo_terminating_zero_for_lexeme)(&me->buffer);
    QUEX_NAME(Buffer_move_backward)(&me->buffer, CharacterN);
}

QUEX_INLINE void  
QUEX_NAME(undo)(QUEX_TYPE_ANALYZER* me)
{
    __QUEX_IF_COUNT_LINES(me->counter._line_number_at_end     = me->counter._line_number_at_begin);
    __QUEX_IF_COUNT_COLUMNS(me->counter._column_number_at_end = me->counter._column_number_at_begin);

    QUEX_NAME(Buffer_undo_terminating_zero_for_lexeme)(&me->buffer);

    me->buffer._input_p = me->buffer._lexeme_start_p;
}

#if ! defined(__QUEX_OPTION_PLAIN_C)
QUEX_INLINE size_t  
QUEX_MEMBER(tell)()
{ return QUEX_NAME(tell)(this); }

QUEX_INLINE void    
QUEX_MEMBER(seek)(const size_t CharacterIndex)
{ QUEX_NAME(seek)(this, CharacterIndex); }

QUEX_INLINE void    
QUEX_MEMBER(seek_forward)(const size_t CharacterN)
{ QUEX_NAME(seek_forward)(this, CharacterN); }

QUEX_INLINE void    
QUEX_MEMBER(seek_backward)(const size_t CharacterN)
{ QUEX_NAME(seek_backward)(this, CharacterN); }

QUEX_INLINE void  
QUEX_MEMBER(undo)()
{ return QUEX_NAME(undo)(this); }
#endif

QUEX_NAMESPACE_MAIN_CLOSE

#endif /* __QUEX_INCLUDE_GUARD__ANALYZER__MEMBER__NAVIGATION_I */
