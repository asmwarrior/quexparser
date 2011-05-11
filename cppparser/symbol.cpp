#include "symbol.h"

#define SYMBOL_DEBUG_OUTPUT 0

#if SYMBOL_DEBUG_OUTPUT
    #define TRACE(format, args...)\
    Manager::Get()->GetLogManager()->DebugLog(F( format , ## args))
#else
    #define TRACE(format, ...)
	#define DebugLog(format,...)
#endif

Symbol::Symbol()
{

}
Symbol::Symbol(SymbolKind kind,Token &name)
{
    m_SymbolKind = kind;
    m_Name = name;
}


Symbol::~Symbol()
{
    //dtor
}
//--------------------------------------------------------------------------------------------
SymbolTree::SymbolTree()
{

}

SymbolTree::~SymbolTree()
{
    clear();
}
void SymbolTree::clear()
{
    size_t i;
    for(i = 0;i < m_SymbolList.size(); i++)
    {
        Symbol* symol = m_SymbolList[i];
        if(symol)
            delete symol;
    }
    m_SymbolIdxTree.clear();
}
size_t SymbolTree::insert(Symbol* newSymbol)
{
	m_SymbolList.push_back(newSymbol);
	size_t i = m_SymbolList.size();
	cc_string key = newSymbol->m_Name.get_text();
	SymbolIdxSet &IdxSet =  m_SymbolIdxTree[key];
	IdxSet.insert(i);
	return i;
}
void SymbolTree::erase(Symbol* oldSymbol)
{

}

size_t SymbolTree::FindMatches(const cc_string& key,SymbolIdxSet& result,bool caseSensitive, bool is_prefix, SymbolKind kindMask)
{
    set<size_t> lists;
    result.clear();
    int numitems = m_SymbolIdxTree.FindMatches(key,lists,caseSensitive,is_prefix);
    if(!numitems)
        return 0;
    // now the lists contains indexes to all the matching keywords
    SymbolIdxSet* curset;
    set<size_t>::iterator it;
    SymbolIdxSet::iterator it2;
    // first loop will find all the keywords
    for(it = lists.begin(); it != lists.end(); it++)
    {
        curset = &(m_SymbolIdxTree.GetItemAtPos(*it));
        // second loop will get all the items maped by the same keyword,
        // for example, we have ClassA::foo, ClassB::foo ...
        for(it2 = curset->begin();it2 != curset->end(); it2++)
        {
            if (kindMask == 0xffff || (at(*it)->m_SymbolKind & kindMask))
                result.insert(*it2);
        }
    }
    return result.size();



    return 0;
}

Symbol* SymbolTree::GetSymbolAt(int idx)
{
    if (idx < 0 || (size_t)idx >= m_SymbolIdxTree.size())
        return 0;

    return m_SymbolList[idx];
}

void SymbolTree::Dump()
{
    cc_string text;
    text = m_SymbolIdxTree.dump();
    cout<<text;
}



