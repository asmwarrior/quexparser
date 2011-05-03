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
Symbol::Symbol(SymbolKind kind,RawToken &name)
{
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

size_t SymbolTree::FindMatches(const cc_string& s,SymbolIdxSet& result)
{
    return 0;
}

void SymbolTree::Dump()
{
    cc_string text;
    text = m_SymbolIdxTree.dump();
    cout<<text;
}



