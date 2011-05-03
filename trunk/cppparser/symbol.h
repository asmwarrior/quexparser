#ifndef SYMBOL_H
#define SYMBOL_H

#include "searchtree.h"
#include "tokenizer.h"

using namespace std;


/** one argument
  * for example: void f(int a=0, float b=3);
  * then there are two SingleArgument
  * "int   a 0"
  * "float b 3"
  */
struct SingleArgument
{
    RawToken type;
    RawToken name;
    RawToken defaultValue;
};
typedef std::vector<RawToken> ArgumentList;

/** for a template type definition, we have such format
  *   AAA<typename T1 = int, class T2 = float>::
  */
struct ScopeBlock
{
    RawToken     name;
    ArgumentList templateArgumentList;
};

/** AAA<typename T1 = int, class T2 = float>::BBB */
typedef std::vector<ScopeBlock> ScopeQueue;

class Symbol;
class SymbolTree;

typedef vector<Symbol*> SymbolList;
typedef set<int, less<int> > SymbolIdxSet;
typedef SearchTree<SymbolIdxSet> SymbolSearchTree;




enum SymbolScope
{
    tsUndefined = 0,
    tsPrivate,
    tsProtected,
    tsPublic
};

enum SymbolKind
{

    tkUndefined        = 0,
    tkNamespace           ,
    tkClass               ,
    tkTemplateClass       ,
    tkEnum                ,
    tkTypedef             ,
    tkConstructor         ,
    tkDestructor          ,
    tkFunction            ,
    tkTemplateFunction    ,
    tkVariable            ,
    tkEnumerator          ,
    tkMacroDefine         ,
    tkMacroUsage 	      ,
    tkUsingNamespace      ,
    tkFor                 ,
    tkWhile               ,
    tkTry                 ,
    tkDoWhile             ,
    tkParameter

};

struct DeclarationModifier
{
    bool isConst;
    bool isConstructor;
    bool isDestructor;
    bool isVirtual;
    bool isOperator;
    bool isTemplate;
    bool isAuto;
    bool isRegister;
    bool isExtern;
    bool isExplicit;
    bool isFriend;
    bool isVolatile;
    bool isStatic;
    bool isInline;
    bool isMutable;
    bool isPointer;
    bool isReference;
};


class Symbol
{
       friend class SymbolTree;
public:
        Symbol();
        Symbol(SymbolKind kind,RawToken&name);
        ~Symbol();

        //void AddChild(int child);
        //void EraseChild(int child);

        //Symbol* GetParentSymbol();

		ScopeQueue m_NameQueue;
		ScopeQueue m_TypeQueue;
		RawToken   m_Name;

        ArgumentList m_FunctionArgument;
		ArgumentList m_TemplateArgument;

        unsigned int m_FileIdx;
        unsigned int m_Line;
        unsigned int m_ImplFile;
        unsigned int m_ImplLine; // where the token was met
        unsigned int m_ImplLineStart; // if token is impl, opening brace line
        unsigned int m_ImplLineEnd; // if token is impl, closing brace line

        SymbolScope m_Scope;
        SymbolKind  m_SymbolKind;

        int m_ParentIndex;

        SymbolIdxSet m_Children;
        SymbolIdxSet m_Ancestor;
        SymbolIdxSet m_DirectAncestor;
        SymbolIdxSet m_Descendant;

        int m_Self; // current index in the tree
};

class SymbolTree
{
public:
        SymbolTree();
		~SymbolTree();

        // STL compatibility functions
        void clear();

        size_t insert(Symbol* newSymbol);
        void erase(Symbol* oldSymbol);

		size_t FindMatches(const cc_string& s,SymbolIdxSet& result);
		void Dump();
private:
        SymbolList m_SymbolList; /// Contains the pointers to all the tokens
        SymbolSearchTree m_SymbolIdxTree; /** Tree containing the indexes to the tokens(the indexes will be used on m_Symbol) */

};


#endif
