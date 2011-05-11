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
    Token type;
    Token name;
    Token defaultValue;
};
typedef std::vector<Token> ArgumentList;

/** for a template type definition, we have such format
  *   AAA<typename T1 = int, class T2 = float>::
  */
struct ScopeBlock
{
    Token     name;
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
    tkNamespace        = 1,
    tkClass            = 1<<1,
    tkTemplateClass    = 1<<2,
    tkEnum             = 1<<3,
    tkTypedef          = 1<<4,
    tkConstructor      = 1<<5,
    tkDestructor       = 1<<6,
    tkFunction         = 1<<7,
    tkTemplateFunction = 1<<8,
    tkVariable         = 1<<9,
    tkEnumerator       = 1<<10,
    tkMacroDefine      = 1<<11,
    tkMacroUsage 	   = 1<<12,
    tkUsingNamespace   = 1<<13,
    tkFor              = 1<<14,
    tkWhile            = 1<<15,
    tkTry              = 1<<16,
    tkDoWhile          = 1<<17,
    tkParameter        = 1<<18

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
        Symbol(SymbolKind kind,Token&name);
        ~Symbol();

        //void AddChild(int child);
        //void EraseChild(int child);

        //Symbol* GetParentSymbol();

		ScopeQueue m_NameQueue;
		ScopeQueue m_TypeQueue;
		Token   m_Name;

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
        inline Symbol* operator[](int idx) { return GetSymbolAt(idx); }
        inline Symbol* at(int idx)         { return GetSymbolAt(idx); }
        Symbol* GetSymbolAt(int idx);

        size_t insert(Symbol* newSymbol);
        void erase(Symbol* oldSymbol);

		size_t FindMatches(const cc_string& s,SymbolIdxSet& result,bool caseSensitive, bool is_prefix, SymbolKind kindMask);
		void Dump();
private:
        SymbolList m_SymbolList; /// Contains the pointers to all the tokens
        SymbolSearchTree m_SymbolIdxTree; /** Tree containing the indexes to the tokens(the indexes will be used on m_Symbol) */

};


#endif
