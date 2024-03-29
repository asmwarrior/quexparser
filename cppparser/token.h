/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef TOKEN_H
#define TOKEN_H

#include "searchtree.h"
#include "tokenizer.h"

using namespace std;

class Token;
class TokensTree;

static wxCriticalSection s_MutexProtection;
enum FileParsingStatus
{
    fpsNotParsed = 0,
    fpsAssigned,
    fpsBeingParsed,
    fpsDone
};


typedef vector<Token*> TokenList;

typedef deque<int> TokenIdxList;
typedef set<int, less<int> > TokenIdxSet;
typedef SearchTree<TokenIdxSet> TokenSearchTree;
typedef BasicSearchTree TokenFilenamesMap;
typedef map<size_t,TokenIdxSet,less<size_t> > TokenFilesMap;
typedef map<size_t,FileParsingStatus,less<size_t> > TokenFilesStatus;
typedef set<size_t,less<size_t> > TokenFilesSet;

enum TokenScope
{
    tsUndefined = 0,
    tsPrivate,
    tsProtected,
    tsPublic
};

enum TokenKind
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
    tkUsingNamespace

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
//typedef std::vector<cc_string> ArgumentList;
typedef cc_string ArgumentList;

/** for a template type definition, we have such format
  *   AAA<typename T1 = int, class T2 = float>::
  */
struct ScopeBlock
{
    RawToken     name;
    ArgumentList templateArgumentList;
};



typedef std::vector<ScopeBlock> FullIdentifier;




class Token
{
    friend class TokensTree;
    public:
        Token();
        Token(const cc_string& name, unsigned int file, unsigned int line);
        ~Token();

        void AddChild(int child);
        void EraseChild(int child);
        cc_string GetNamespace() const;
        bool InheritsFrom(int idx) const;
        cc_string DisplayName() const;
        cc_string GetTokenKindString() const;
        cc_string GetTokenScopeString() const;
        cc_string GetFilename() const;
        cc_string GetImplFilename() const;
        inline unsigned long GetTicket() const { return m_Ticket; }
        bool MatchesFiles(const TokenFilesSet& files);

        bool SerializeIn(std::istream& f);
        bool SerializeOut(std::ostream& f);

        int GetSelf() { return m_Self; } // current index in the tree
        cc_string GetParentName();
        Token* GetParentToken();
        TokensTree* GetTree() { return m_pTree; }

        cc_string m_Type; // this is the return value (if any): e.g. const string&
        cc_string m_ActualType; // this is what the parser believes is the actual return value: e.g. string
        cc_string m_Name;

        /// full type information has the format like  A<a,b>::B<d,f>::C
        FullIdentifier m_FullType;

        /// full name information could has the same string as full type
        FullIdentifier m_FullName;

        /// function argument list, hopefully we only get type information
        ArgumentList m_Args;

        cc_string m_RealArgs;

        cc_string m_AncestorsString; // all ancestors comma-separated list

        unsigned int m_FileIdx;
        unsigned int m_Line;
        unsigned int m_ImplFile;
        unsigned int m_ImplLine; // where the token was met
        unsigned int m_ImplLineStart; // if token is impl, opening brace line
        unsigned int m_ImplLineEnd; // if token is impl, closing brace line
        TokenScope m_Scope;
        TokenKind m_TokenKind;
        bool m_IsOperator;
        bool m_IsLocal; // found in a local file?
        bool m_IsTemp; // if true, the tree deletes it in FreeTemporaries()
        //bool m_IsConst;    // the member method is const (yes/no)

        int m_ParentIndex;
        TokenIdxSet m_Children;
        TokenIdxSet m_Ancestors;
        TokenIdxSet m_DirectAncestors;
        TokenIdxSet m_Descendants;

        //vector<cc_string> m_Aliases; // used for namespace aliases

        void* m_pUserData; // custom user-data (the classbrowser expects it to be a pointer to a cbProject)
    protected:
        TokensTree* m_pTree;
        int m_Self; // current index in the tree
        unsigned long m_Ticket;
        static unsigned long GetTokenTicket();

    private:
};

class TokensTree
{
    public:
        static const cc_string s_version;
        TokensTree();
        // STL compatibility functions
        void clear();
        inline Token* operator[](int idx) { return GetTokenAt(idx); }
        inline Token* at(int idx) { return GetTokenAt(idx); }
        size_t size();
        size_t realsize();
        inline bool empty() { return size()==0; }
        int insert(Token* newToken);
        int insert(int loc, Token* newToken);
        int erase(int loc);
        void erase(Token* oldToken);

        // Token specific functions
        void RecalcFreeList();
        void RecalcData();
        int TokenExists(const cc_string& name, int parent, short int kindMask);
        size_t FindMatches(const cc_string& s,TokenIdxSet& result,bool caseSensitive,bool is_prefix, int kindMask = 0xffff);
        size_t FindTokensInFile(const cc_string& file, TokenIdxSet& result, short int kindMask);
        void eraseFile(const cc_string& filename);
        void eraseFile(int index);
        void FreeTemporaries();
        virtual ~TokensTree();

        // Parsing related functions

        size_t GetFileIndex(const cc_string& filename);
        const cc_string GetFilename(size_t idx) const;
        size_t ReserveFileForParsing(const cc_string& filename,bool preliminary = false);
        void FlagFileForReparsing(const cc_string& filename);
        void FlagFileAsParsed(const cc_string& filename);
        bool IsFileParsed(const cc_string& filename);

        void MarkFileTokensAsLocal(const cc_string& filename, bool local = true, void* userData = 0);
        void MarkFileTokensAsLocal(size_t file, bool local = true, void* userData = 0);

        TokenList m_Tokens; /// Contains the pointers to all the tokens
        TokenSearchTree m_Tree; /** Tree containing the indexes to the tokens
          (the indexes will be used on m_Tokens) */

        TokenFilenamesMap m_FilenamesMap; /** Map: filenames -> file indexes */
        TokenFilesMap m_FilesMap; /** Map: file indexes -> Sets of TokenIndexes */
        TokenFilesSet m_FilesToBeReparsed; /** Set: file indexes */
        TokenIdxList m_FreeTokens; /** List of all the deleted (and available) tokens */

        /** List of tokens belonging to the global namespace */
        TokenIdxSet m_TopNameSpaces,
        m_GlobalNameSpace;

        TokenFilesStatus m_FilesStatus; /** Parse Status for each file */
        bool m_modified;
    protected:
        Token* GetTokenAt(int idx);
        int AddToken(Token* newToken,int forceidx = -1);

        void eraseToken(int idx);
        void eraseToken(Token* oldToken);

        int AddTokenToList(Token* newToken,int forceidx = -1);
        void eraseTokenFromList(int idx);

        void RecalcFullInheritance(int parentIdx, TokenIdxSet& result); // called by RecalcData
};


/*
inline void SaveIntToFile(wxOutputStream* f, int i)
{
    // This used to be done as        f->Write(&i, sizeof(int));
    //which is incorrect because it assumes a consistant byte order
    //and a constant int size

    unsigned int const j = i; // rshifts aren't well-defined for negatives
    char c[4] = { (char) (j>>0&0xFF), (char) (j>>8&0xFF), (char) (j>>16&0xFF), (char) (j>>24&0xFF) };
    f->Write( c, 4 );
}

inline bool LoadIntFromFile(wxInputStream* f, int* i)
{
//    See SaveIntToFile
//    return f->Read(i, sizeof(int)) == sizeof(int);

    char c[4];
    if ( f->Read( c, 4 ).LastRead() != 4 ) return false;
    *i = ( c[0]<<0 | c[1]<<8 | c[2]<<16 | c[3]<<24 );
    return true;
}

inline void SaveStringToFile(wxOutputStream* f, const string& str)
{
    const wxWX2MBbuf psz = str.mb_str(wxConvUTF8);
    // TODO (MortenMacFly#5#): Can we safely use strlen here?
    int size = psz ? strlen(psz) : 0;
    if (size >= 32767)
        size = 32767;
    SaveIntToFile(f, size);
    if(size)
        f->Write(psz, size);
}

inline bool LoadStringFromFile(wxInputStream* f, string& str)
{
    int size;
    if (!LoadIntFromFile(f, &size))
        return false;
    bool ok = true;
    if (size > 0 && size <= 32767)
    {
        char buf[32768];
        ok = f->Read(buf, size).LastRead() == (size_t)size;
        buf[size] = '\0';
        str = string(buf, wxConvUTF8);
    }
    else // doesn't fit in our buffer, but still we have to skip it
    {
        str.Empty();
        size = size & 0xFFFFFF; // Can't get any longer than that
        f->SeekI(size, wxFromCurrent);
    }
    return ok;
}
*/


#endif // TOKEN_H
