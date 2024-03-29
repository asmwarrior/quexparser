/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include "token.h"

#define TOKEN_DEBUG_OUTPUT 0

#if TOKEN_DEBUG_OUTPUT
    #define TRACE(format, args...)\
    Manager::Get()->GetLogManager()->DebugLog(F( format , ## args))
#else
    #define TRACE(format, ...)
	#define DebugLog(format,...)
#endif

inline void stringToArray(const cc_string & text, std::vector<cc_string> & array, const cc_string  & separator = cc_text(""), bool trimSpaces = true)
{
    cc_string search = text;
    int seplen = cc_string(separator).length();
    while (true)
    {
        int idx = search.find(separator);
        if (idx == -1)
        {
            if (trimSpaces)
            {
                cc_trimAll(search);
            }
            if (!search.empty())
                array.push_back(search);
            break;
        }
        cc_string part = search.substr(0,idx);
        search.erase(0, idx + seplen);
        if (trimSpaces)
        {
            cc_trimAll(part);
        }
        if (!part.empty())
            array.push_back(part);
    }
}


class StringTokenizer
{
public:
    StringTokenizer(const cc_string & str, const cc_string & szTokens)
	{
		stringToArray(str,m_array,szTokens);
		m_index = 0;
	}
        cc_string GetNextToken()
	{
        if (m_index < 0 || m_index >= m_array.size())
            return cc_text("");
		return m_array[m_index++];
	}
    bool HasMoreTokens()
	{
        return m_index < m_array.size();
	}

protected:
    std::vector<cc_string>	m_array;
    unsigned int		m_index;
};


inline void SaveTokenIdxSetToFile(std::ostream& f,const TokenIdxSet& data)
{
    SaveIntToFile(f, (int)(data.size()));
    for (TokenIdxSet::iterator it = data.begin(); it != data.end(); it++)
    {
        int num = *it;
        SaveIntToFile(f, num);
    }
}

bool LoadTokenIdxSetFromFile(std::istream& f,TokenIdxSet* data)
{
    if(!data)
        return false;
    bool result = true;
    data->clear();
    int size = 0;
    do
    {
        if (!LoadIntFromFile(f, &size))
        {
            result = false;
            break;
        }
        int i,num = 0;
        for (i = 0; i < size; i++)
        {
            if(!LoadIntFromFile(f,&num))
            {
                result = false;
                break;
            }
            data->insert(num);
        }
    }while(false);
    return result;
}


Token::Token()
    :
    m_FileIdx(0),
    m_Line(0),
    m_ImplFile(0),
    m_ImplLine(0),
    m_ImplLineStart(0),
    m_ImplLineEnd(0),
    m_Scope(tsUndefined),
    m_TokenKind(tkUndefined),
    m_IsLocal(false),
    m_IsTemp(false),
    m_ParentIndex(-1),
    m_pUserData(0),
    m_pTree(0),
    m_Self(-1)
{
    m_Ticket = GetTokenTicket();
}

Token::Token(const cc_string& name, unsigned int file, unsigned int line)
    : m_Name(name),
    m_FileIdx(file),
    m_Line(line),
    m_ImplFile(0),
    m_ImplLine(0),
    m_ImplLineStart(0),
    m_ImplLineEnd(0),
    m_Scope(tsUndefined),
    m_TokenKind(tkUndefined),
    m_IsLocal(false),
    m_IsTemp(false),
    m_ParentIndex(-1),
    m_pUserData(0),
    m_pTree(0),
    m_Self(-1)
{
    //ctor
    m_Ticket = GetTokenTicket();

}

Token::~Token()
{
    //dtor
}

unsigned long Token::GetTokenTicket()
{
    static wxCriticalSection s_TicketProtection;
    static unsigned long ticket = 256; // Reserve some space for the class browser
    wxCriticalSectionLocker lock(s_TicketProtection);
    return ticket++;
}

cc_string Token::GetParentName()
{
    cc_string parentname = cc_text("");
    wxCriticalSectionLocker* lock = 0;
    if(m_pTree)
        lock = new wxCriticalSectionLocker(s_MutexProtection);
    Token* parent = GetParentToken();
    if(parent)
        parentname = parent->m_Name;
    if(lock)
        delete lock;
    return parentname;
}

cc_string Token::DisplayName() const
{
    cc_string result;
    if      (m_TokenKind == tkClass)
        return result << cc_text("class ")     << m_Name << m_RealArgs << cc_text(" {...}");
    else if (m_TokenKind == tkNamespace)
        return result << cc_text("namespace ") << m_Name << cc_text(" {...}");
    else if (m_TokenKind == tkEnum)
        return result << cc_text("enum ")      << m_Name << cc_text(" {...}");
    else if (m_TokenKind == tkTypedef)
    {
        result << cc_text("typedef");

        if (!m_Type.empty())
            result << cc_text(" ") << m_Type;

                if (result.rfind('*') != cc_string::npos)
        {
            result.erase(result.end()-1,result.end());
            return result << m_Name << cc_text(")") <<  m_Args;
        }
        return result << cc_text(" ") << m_Name;
    }
    else if (m_TokenKind == tkMacroDefine)
    {
        result << cc_text("#define ") << m_Name << m_Args;
        if (!m_Type.empty())
            return result << cc_text(" ") << m_Type;
    }

    // else
    if (!m_Type.empty())
        result << m_Type << cc_text(" ");

	if (m_TokenKind == tkEnumerator)
                return 	result << GetNamespace() << m_Name << cc_text("=") << m_Args;

    return result << GetNamespace() << m_Name << m_Args;
}

Token* Token::GetParentToken()
{
    Token* the_token = 0;
    if(!m_pTree)
        return 0;
    the_token = m_pTree->at(m_ParentIndex);
    return the_token;
}

cc_string Token::GetFilename() const
{
    if(!m_pTree)
        return cc_string(cc_text(""));
    return m_pTree->GetFilename(m_FileIdx);
}

cc_string Token::GetImplFilename() const
{
    if(!m_pTree)
        return cc_string(cc_text(""));
    return m_pTree->GetFilename(m_ImplFile);
}

bool Token::MatchesFiles(const TokenFilesSet& files)
{
    if(!files.size())
        return true;
    if(!m_FileIdx && !m_ImplFile)
        return true;
    if((m_FileIdx && files.count(m_FileIdx)) || (m_ImplFile && files.count(m_ImplFile)))
        return true;
    return false;
}

cc_string Token::GetNamespace() const
{
    const cc_string dcolon(cc_text("::"));
    cc_string res;
    Token* parentToken = m_pTree->at(m_ParentIndex);
    while (parentToken)
    {
        res.insert(0,dcolon);
        res.insert(0,parentToken->m_Name);
        parentToken = parentToken->GetParentToken();
    }
    return res;
}

void Token::AddChild(int child)
{
    if (child >= 0)
        m_Children.insert(child);
}

bool Token::InheritsFrom(int idx) const
{
    if (idx < 0 || !m_pTree)
        return false;
    Token* token = m_pTree->at(idx);
    if(!token)
        return false;

    for (TokenIdxSet::iterator it = m_DirectAncestors.begin(); it != m_DirectAncestors.end(); it++)
    {
        int idx2 = *it;
        Token* ancestor = m_pTree->at(idx2);
        if(!ancestor)
            continue;
        if (ancestor == token || ancestor->InheritsFrom(idx))  // ##### is this intended?
            return true;
    }
    return false;
}

cc_string Token::GetTokenKindString() const
{
    switch (m_TokenKind)
    {
        case tkClass:            return cc_text("class");
        case tkTemplateClass:    return cc_text("template class");
        case tkNamespace:        return cc_text("namespace");
        case tkTypedef:          return cc_text("typedef");
        case tkEnum:             return cc_text("enum");
        case tkEnumerator:       return cc_text("enumerator");
        case tkFunction:         return cc_text("function");
        case tkTemplateFunction: return cc_text("template function");
        case tkConstructor:      return cc_text("constructor");
        case tkDestructor:       return cc_text("destructor");
        case tkMacroDefine:      return cc_text("macro define");
        case tkMacroUsage:       return cc_text("macro usage");
        case tkVariable:         return cc_text("variable");
        case tkUsingNamespace:   return cc_text("using namespace");
        default:                 return cc_text("undefined");
    }

}

cc_string Token::GetTokenScopeString() const
{
    switch (m_Scope)
    {
        case tsPrivate:   return cc_text("private");
        case tsProtected: return cc_text("protected");
        case tsPublic:    return cc_text("public");
        default:          return cc_text("");
    }
}

bool Token::SerializeIn(std::istream& f)
{
    bool result = true;
    do
    {
        if (!LoadIntFromFile(f, (int*)&m_Self))
        {
            result = false;
            break;
        }
        if (!LoadIntFromFile(f, (int*)&m_ParentIndex))
        {
            result = false;
            break;
        }
        if(m_ParentIndex < 0)
            m_ParentIndex = -1;

        if (!LoadStringFromFile(f, m_Type))
        {
            result = false;
            break;
        }
        if (!LoadStringFromFile(f, m_ActualType))
        {
            result = false;
            break;
        }
        if (!LoadStringFromFile(f, m_Name))
        {
            result = false;
            break;
        }
        if (!LoadStringFromFile(f, m_Args))
        {
            result = false;
            break;
        }
        if (!LoadStringFromFile(f, m_AncestorsString))
        {
            result = false;
            break;
        }
        if (!LoadIntFromFile(f, (int*)&m_FileIdx))
        {
            result = false;
            break;
        }
        if (!LoadIntFromFile(f, (int*)&m_Line))
        {
            result = false;
            break;
        }
        if (!LoadIntFromFile(f, (int*)&m_ImplFile))
        {
            result = false;
            break;
        }
        if (!LoadIntFromFile(f, (int*)&m_ImplLine))
        {
            result = false;
            break;
        }
        if (!LoadIntFromFile(f, (int*)&m_Scope))
        {
            result = false;
            break;
        }
        if (!LoadIntFromFile(f, (int*)&m_TokenKind))
        {
            result = false;
            break;
        }
        if (!LoadIntFromFile(f, (int*)&m_IsOperator))
        {
            result = false;
            break;
        }
        if (!LoadIntFromFile(f, (int*)&m_IsLocal))
        {
            result = false;
            break;
        }
        if(!LoadTokenIdxSetFromFile(f, &m_Ancestors))
        {
            result = false;
            break;
        }
        if(!LoadTokenIdxSetFromFile(f, &m_Children))
        {
            result = false;
            break;
        }
        if(!LoadTokenIdxSetFromFile(f, &m_Descendants))
        {
            result = false;
            break;
        }
    } while(false);
    return result;
}

bool Token::SerializeOut(std::ostream& f)
{
    SaveIntToFile(f, m_Self);
    SaveIntToFile(f, m_ParentIndex);
    SaveStringToFile(f, m_Type);
    SaveStringToFile(f, m_ActualType);
    SaveStringToFile(f, m_Name);
    SaveStringToFile(f, m_Args);
    SaveStringToFile(f, m_AncestorsString);
    SaveIntToFile(f, m_FileIdx);
    SaveIntToFile(f, m_Line);
    SaveIntToFile(f, m_ImplFile);
    SaveIntToFile(f, m_ImplLine);
    SaveIntToFile(f, m_Scope);
    SaveIntToFile(f, m_TokenKind);
    SaveIntToFile(f, m_IsOperator ? 1 : 0);
    SaveIntToFile(f, m_IsLocal ? 1 : 0);

    SaveTokenIdxSetToFile(f, m_Ancestors);
    SaveTokenIdxSetToFile(f, m_Children);
    SaveTokenIdxSetToFile(f, m_Descendants);
    return true;
}

// *** TokensTree ***

TokensTree::TokensTree():
m_modified(false)
{
    m_Tokens.clear();
    m_FilenamesMap.clear();
    m_FilesMap.clear();
    m_FilesStatus.clear();
    m_FreeTokens.clear();
    m_FilesToBeReparsed.clear();
    m_TopNameSpaces.clear();
    m_GlobalNameSpace.clear();
}


TokensTree::~TokensTree()
{
    clear();
}

void TokensTree::clear()
{
    m_Tree.clear();
    m_FilenamesMap.clear();
    m_FilesMap.clear();
    m_FilesStatus.clear();
    m_FilesToBeReparsed.clear();
    m_FreeTokens.clear();
    m_TopNameSpaces.clear();
    m_GlobalNameSpace.clear();

    size_t i;
    for(i = 0;i < m_Tokens.size(); i++)
    {
        Token* token = m_Tokens[i];
        if(token)
            delete token;
    }
    m_Tokens.clear();
}

size_t TokensTree::size()
{
    return m_Tokens.size();
}

size_t TokensTree::realsize()
{
    if(m_Tokens.size() <= m_FreeTokens.size())
        return 0;
    return m_Tokens.size() - m_FreeTokens.size();
}

int TokensTree::insert(Token* newToken)
{
    if(!newToken)
        return -1;
    return AddToken(newToken, -1);
}

int TokensTree::insert(int loc, Token* newToken)
{
    if(!newToken)
        return -1;

    return AddToken(newToken, loc);
}

int TokensTree::erase(int loc)
{
    if(!m_Tokens[loc])
        return 0;
    eraseToken(loc);
    return 1;
}

void TokensTree::erase(Token* oldToken)
{
    eraseToken(oldToken);
}

int TokensTree::TokenExists(const cc_string& name, int parent, short int kindMask)
{
    int idx = m_Tree.GetItemIdx(name);
    if(!idx)
        return -1;
    TokenIdxSet::iterator it;
    TokenIdxSet& curlist = m_Tree.GetItemAtPos(idx);
    int result = -1;
    for(it = curlist.begin(); it != curlist.end(); it++)
    {
        result = *it;
        if(result < 0 || (size_t)result >= m_Tokens.size())
            continue;
        Token* curtoken = m_Tokens[result];
        if(!curtoken)
            continue;
        if((parent < 0 || curtoken->m_ParentIndex == parent) && curtoken->m_TokenKind & kindMask)
            return result;
    }
    return -1;
}

size_t TokensTree::FindMatches(const cc_string& s,TokenIdxSet& result,bool caseSensitive,bool is_prefix, int kindMask)
{
    set<size_t> lists;
    result.clear();
    int numitems = m_Tree.FindMatches(s,lists,caseSensitive,is_prefix);
    if(!numitems)
        return 0;
    // now the lists contains indexes to all the matching keywords
    TokenIdxSet* curset;
    set<size_t>::iterator it;
    TokenIdxSet::iterator it2;
    // first loop will find all the keywords
    for(it = lists.begin(); it != lists.end(); it++)
    {
        curset = &(m_Tree.GetItemAtPos(*it));
        // second loop will get all the items maped by the same keyword,
        // for example, we have ClassA::foo, ClassB::foo ...
        for(it2 = curset->begin();it2 != curset->end(); it2++)
        {
            if (kindMask == 0xffff || (at(*it)->m_TokenKind & kindMask))
                result.insert(*it2);
        }
    }
    return result.size();
}

size_t TokensTree::FindTokensInFile(const cc_string& file, TokenIdxSet& result, short int kindMask)
{
    result.clear();

    // get file idx
    if (!m_FilenamesMap.HasItem(file))
        return 0;
    int idx = m_FilenamesMap.GetItemIdx(file);

    // now get the tokens set matching this file idx
    TokenFilesMap::iterator itf = m_FilesMap.find(idx);
    if (itf == m_FilesMap.end())
        return 0;
    TokenIdxSet& tokens = itf->second;

    // loop all results and add to final result set after filtering on token kind
    for (TokenIdxSet::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        Token* token = at(*it);
        if (kindMask & token->m_TokenKind)
            result.insert(*it);
    }
    return result.size();
}

int TokensTree::AddToken(Token* newToken,int forceidx)
{
    if(!newToken)
        return -1;

    const cc_string & name = newToken->m_Name;

    static TokenIdxSet tmp_tokens = TokenIdxSet();
    // tmp_tokens.clear();

    // Insert the token's name and the token in the (inserted?) list
    size_t idx2 = m_Tree.AddItem(name,tmp_tokens,false);
    TokenIdxSet& curlist = m_Tree.GetItemAtPos(idx2);

    int newitem = AddTokenToList(newToken,forceidx);
    curlist.insert(newitem);
    m_FilesMap[newToken->m_FileIdx].insert(newitem);

    // Add Token (if applicable) to the namespaces indexes
    if(newToken->m_ParentIndex < 0)
    {
        newToken->m_ParentIndex = -1;
        m_GlobalNameSpace.insert(newitem);
        if(newToken->m_TokenKind == tkNamespace)
            m_TopNameSpaces.insert(newitem);
    }

    // All done!
    return newitem;
}

void TokensTree::eraseToken(int idx)
{
    if(idx<0 || (size_t)idx >= m_Tokens.size())
        return;
    eraseToken(m_Tokens[idx]);
}

void TokensTree::eraseToken(Token* oldToken)
{
    if(!oldToken)
        return;
    int idx = oldToken->m_Self;
    if(m_Tokens[idx]!=oldToken)
        return;

    // Step 1: Detach token from its parent

    Token* parentToken = 0;
    if((size_t)(oldToken->m_ParentIndex) >= m_Tokens.size())
        oldToken->m_ParentIndex = -1;
    if(oldToken->m_ParentIndex >= 0)
        parentToken = m_Tokens[oldToken->m_ParentIndex];
    if(parentToken)
        parentToken->m_Children.erase(idx);

    TokenIdxSet nodes;
    TokenIdxSet::iterator it;

    // Step 2: Detach token from its ancestors

    nodes = (oldToken->m_DirectAncestors);
    for(it = nodes.begin();it!=nodes.end(); it++)
    {
        int ancestoridx = *it;
        if(ancestoridx < 0 || (size_t)ancestoridx >= m_Tokens.size())
            continue;
        Token* ancestor = m_Tokens[ancestoridx];
        if(ancestor)
            ancestor->m_Descendants.erase(idx);
    }
    oldToken->m_Ancestors.clear();
    oldToken->m_DirectAncestors.clear();

    // Step 3: erase children

    nodes = (oldToken->m_Children); // Copy the list to avoid interference
    for(it = nodes.begin();it!=nodes.end(); it++)
        eraseToken(*it);
    // m_Children SHOULD be empty by now - but clear anyway.
    oldToken->m_Children.clear();

    // Step 4: erase descendants

    nodes = oldToken->m_Descendants; // Copy the list to avoid interference
    for(it = nodes.begin();it!=nodes.end(); it++)
    {
        if(*it == idx) // that should not happen, we can not be our own descendant, but in fact that can happen with boost
        {
            DebugLog(cc_text("Break out the loop to erase descendants, to avoid a crash. We can not be our own descendant !!"));
            break;
        }
         eraseToken(*it);
    }
    // m_Descendants SHOULD be empty by now - but clear anyway.
    oldToken->m_Descendants.clear();

    // Step 5: Detach token from the SearchTrees

    int idx2 = m_Tree.GetItemIdx(oldToken->m_Name);
    if(idx2)
    {
        TokenIdxSet& curlist = m_Tree.GetItemAtPos(idx2);
        curlist.erase(idx);
    }

    // Now, from the global namespace (if applicable)
    if(oldToken->m_ParentIndex == -1)
    {
        m_GlobalNameSpace.erase(idx);
        m_TopNameSpaces.erase(idx);
    }

    // Step 6: Finally, erase it from the list.

    eraseTokenFromList(idx);
}

int TokensTree::AddTokenToList(Token* newToken,int forceidx)
{
    int result = -1;
    if(!newToken)
        return -1;
    if(forceidx >= 0) // Reading from Cache?
    {
        if((size_t)forceidx >= m_Tokens.size())
        {
            int max = 250*((forceidx + 250) / 250);
            m_Tokens.resize((max),0); // fill next 250 items with null-values
        }
        m_Tokens[forceidx] = newToken;
        result = forceidx;
    }
    else // For Realtime Parsing
    {
        if(m_FreeTokens.size())
        {
            result = m_FreeTokens[m_FreeTokens.size() - 1];
            m_FreeTokens.pop_back();
            m_Tokens[result] = newToken;
        }
        else
        {
            result = m_Tokens.size();
            m_Tokens.push_back(newToken);
        }
    }

    newToken->m_pTree = this;
    newToken->m_Self = result;
    // Clean up extra string memory
/*  Shrink = resize(size())
    newToken->m_Type.Shrink();
    newToken->m_Name.Shrink();
    newToken->m_Args.Shrink();
    newToken->m_AncestorsString.Shrink();
*/
    return result;
}

void TokensTree::eraseTokenFromList(int idx)
{
    if(idx < 0 || (size_t)idx >= m_Tokens.size())
        return;
    Token* oldToken = m_Tokens[idx];
    if(oldToken)
    {
        m_Tokens[idx] = 0;
        m_FreeTokens.push_back(idx);
        m_FilesToBeReparsed.insert(oldToken->m_FileIdx);
        delete oldToken;
    }
}

void TokensTree::eraseFile(const cc_string& filename)
{
    int index = GetFileIndex(filename);
    eraseFile(index);
}

void TokensTree::eraseFile(int index)
{
    if(index <=0)
        return;
    TokenIdxSet& the_list = m_FilesMap[index];
    TokenIdxSet::iterator it;
    for(it = the_list.begin(); it != the_list.end();it++)
    {
        int idx = *it;
        if(idx < 0 || (size_t)idx > m_Tokens.size())
            continue;
        Token* the_token = at(idx);

        if(!the_token)
            continue;

        // do not erase token lightly...
        // only if both its decl filename and impl filename are either empty or match this file
        // if one of those filenames do not match the above criteria
        // just clear the relevant info, leaving the token where it is...

        bool match1 = the_token->m_FileIdx == 0 || (int)the_token->m_FileIdx == index;
        bool match2 = the_token->m_ImplFile == 0 || (int)the_token->m_ImplFile == index;
        if (match1 && match2)
            eraseToken(the_token); // safe to erase the token
        else
        {
            // do not erase token, just clear the matching info
            if (match1)
            {
                the_token->m_FileIdx = 0;
                the_token->m_Line = 0;
            }
            else if (match2)
            {
                the_token->m_ImplFile = 0;
                the_token->m_ImplLine = 0;
            }
        }
    }
    the_list.clear();
}

void TokensTree::FreeTemporaries()
{
    int i;
    for(i = m_Tokens.size() -1;i >= 0;i--)
    {
        Token* token = m_Tokens[i];
        if (token && token->m_IsTemp)
            eraseToken(token);
    }
}

void TokensTree::RecalcFreeList()
{
    m_FreeTokens.clear();
    int i;
    for(i = m_Tokens.size() -1;i >= 0;i--)
    {
        if(!m_Tokens[i])
            m_FreeTokens.push_back(i);
    }
}

void TokensTree::RecalcData()
{
    TRACE(cc_text("RecalcData() : Calculating full inheritance tree."));
    // first loop to convert ancestors string to token indices for each token
    for (size_t i = 0; i < size(); ++i)
    {
        Token* token = at(i);
        if (!token)
            continue;

        if (!(token->m_TokenKind & (tkClass | tkTypedef | tkEnum)))
            continue;
        if (token->m_AncestorsString.empty())
            continue;
        // only local symbols might change inheritance
//        if (!token->m_IsLocal)
//            continue;

        token->m_DirectAncestors.clear();
        token->m_Ancestors.clear();

        TRACE(cc_text("RecalcData() : Token %s, Ancestors %s"),
              token->m_Name.wx_str(),
              token->m_AncestorsString.wx_str());

        StringTokenizer tkz(token->m_AncestorsString, cc_text(","));
        while (tkz.HasMoreTokens())
        {
            cc_string ancestor = tkz.GetNextToken();
            if (ancestor.empty() || ancestor == token->m_Name)
                continue;

            TRACE(cc_text("RecalcData() : Ancestor %s"), ancestor.wx_str());

            // ancestors might contain namespaces, e.g. NS::Ancestor
            if (ancestor.find(cc_text("::")) != cc_string::npos)
            {
                Token* ancestorToken = 0;
                StringTokenizer anctkz(ancestor, cc_text("::"));
                while (anctkz.HasMoreTokens())
                {
                    cc_string ns = anctkz.GetNextToken();
                    if (!ns.empty())
                    {
                        int ancestorIdx = TokenExists(ns, ancestorToken ? ancestorToken->GetSelf() : -1, tkNamespace | tkClass | tkTypedef);
                        ancestorToken = at(ancestorIdx);
//                        ancestorToken = token->HasChildToken(ns, tkNamespace | tkClass);
                        if (!ancestorToken) // unresolved
                            break;
                    }
                }
                if (ancestorToken && ancestorToken != token && ancestorToken->m_TokenKind == tkClass)// && !ancestorToken->m_IsTypedef)
                {
                    TRACE(cc_text("RecalcData() : Resolved to %s"), ancestorToken->m_Name.wx_str());
                    token->m_Ancestors.insert(ancestorToken->GetSelf());
                    ancestorToken->m_Descendants.insert(i);
                    TRACE(cc_text("RecalcData() :  + '%s'"), ancestorToken->m_Name.wx_str());

                }
                else
                    TRACE(cc_text("RecalcData() :  ! '%s' (unresolved)"), ancestor.wx_str());
            }
            else // no namespaces in ancestor
            {
                // accept multiple matches for inheritance
                TokenIdxSet result;
                FindMatches(ancestor, result, true, false);
                for (TokenIdxSet::iterator it = result.begin(); it != result.end(); it++)
                {
                    Token* ancestorToken = at(*it);
                    // only classes take part in inheritance
                    if (   ancestorToken
                        && (ancestorToken != token)
                        && (   (ancestorToken->m_TokenKind == tkClass)
                            || (ancestorToken->m_TokenKind == tkEnum)
                            || (ancestorToken->m_TokenKind == tkTypedef) ) ) // && !ancestorToken->m_IsTypedef)
                    {
                        token->m_Ancestors.insert(*it);
                        ancestorToken->m_Descendants.insert(i);
                        TRACE(cc_text("RecalcData() :  + '%s'"), ancestorToken->m_Name.wx_str());
                    }
                }
#if TOKEN_DEBUG_OUTPUT
                if (result.empty())
                    TRACE(_T("RecalcData() :  ! '%s' (unresolved)"), ancestor.wx_str());
#endif
            }
        }

        token->m_DirectAncestors = token->m_Ancestors;

        if (!token->m_IsLocal) // global symbols are linked once
        {
            TRACE(cc_text("RecalcData() : Removing ancestor string from %s"), token->m_Name.wx_str(), token->m_Name.wx_str());
            token->m_AncestorsString.clear();
        }
    }

    // second loop to calculate full inheritance for each token
    for (size_t i = 0; i < size(); ++i)
    {
        Token* token = at(i);
        if (!token)
            continue;

        if (!(token->m_TokenKind & (tkClass | tkTypedef | tkEnum)))
            continue;

        // recalc
        TokenIdxSet result;
        for (TokenIdxSet::iterator it = token->m_Ancestors.begin(); it != token->m_Ancestors.end(); it++)
            RecalcFullInheritance(*it, result);

        // now, add the resulting set to ancestors set
        for (TokenIdxSet::iterator it = result.begin(); it != result.end(); it++)
        {
            Token* ancestor = at(*it);
            if (ancestor)
            {
                token->m_Ancestors.insert(*it);
                ancestor->m_Descendants.insert(i);
            }
        }

#if TOKEN_DEBUG_OUTPUT
        // debug loop
        TRACE(_T("RecalcData() : Ancestors for %s:"),token->m_Name.wx_str());
        for (TokenIdxSet::iterator it = token->m_Ancestors.begin(); it != token->m_Ancestors.end(); it++)
            TRACE(_T("RecalcData() :  + %s"), at(*it)->m_Name.wx_str());
#endif
    }
    TRACE(cc_text("RecalcData() : Full inheritance calculated."));
}

// caches the inheritance info for each token (recursive function)
void TokensTree::RecalcFullInheritance(int parentIdx, TokenIdxSet& result)
{
    // no parent token? no ancestors...
    if (parentIdx == -1)
        return;

    // no parent token? no ancestors...
    Token* ancestor = at(parentIdx);
    if (!ancestor)
        return;

    // only classes take part in inheritance
    if (!(ancestor->m_TokenKind & (tkClass | tkTypedef)))
        return;

    TRACE(cc_text("RecalcFullInheritance() : Anc: '%s'"), ancestor->m_Name.wx_str());

    // for all its ancestors
    for (TokenIdxSet::iterator it = ancestor->m_Ancestors.begin(); it != ancestor->m_Ancestors.end(); it++)
    {
        if (*it != -1 && // not global scope
            *it != parentIdx && // not the same token (avoid infinite loop)
            result.find(*it) == result.end()) // not already in the set (avoid infinite loop)
        {
            // add it to the set
            result.insert(*it);
            // and recurse for its ancestors
            RecalcFullInheritance(*it, result);
        }
    }
}

Token* TokensTree::GetTokenAt(int idx)
{
    if(idx < 0 || (size_t)idx >= m_Tokens.size())
        return 0;
    return m_Tokens[idx];
}

size_t TokensTree::GetFileIndex(const cc_string& filename)
{
    size_t result = m_FilenamesMap.insert(filename);
    // Insert does not alter the tree if the filename is already found.
    return result;
}

const cc_string TokensTree::GetFilename(size_t idx) const
{
    cc_string result = m_FilenamesMap.GetString(idx);
    return result;
}

bool TokensTree::IsFileParsed(const cc_string& filename)
{
    size_t index = GetFileIndex(filename);
    bool parsed = (m_FilesMap.count(index) &&
                   m_FilesStatus[index]!=fpsNotParsed &&
                  !m_FilesToBeReparsed.count(index)
                  );
    return parsed;
}

void TokensTree::MarkFileTokensAsLocal(const cc_string& filename, bool local, void* userData)
{
    MarkFileTokensAsLocal(GetFileIndex(filename), local, userData);
}

void TokensTree::MarkFileTokensAsLocal(size_t file, bool local, void* userData)
{
    if (file == 0)
        return;

    TokenIdxSet& tokens = m_FilesMap[file];
    for (TokenIdxSet::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        Token* token = m_Tokens.at(*it);
        if (token)
        {
            token->m_IsLocal = local;
            token->m_pUserData = userData;
        }
    }
}

size_t TokensTree::ReserveFileForParsing(const cc_string& filename,bool preliminary)
{
    size_t index = GetFileIndex(filename);
    if (   m_FilesToBeReparsed.count(index)
	    && (  !m_FilesStatus.count(index)
		    || m_FilesStatus[index]==fpsDone) )
    {
        eraseFile(filename);
        m_FilesToBeReparsed.erase(index);
        m_FilesStatus[index]=fpsNotParsed;
    }
    if(m_FilesStatus.count(index))
    {
        FileParsingStatus status = m_FilesStatus[index];
        if(preliminary)
        {
            if(status >= fpsAssigned)
                return 0; // Already assigned
        }
        else
        {
            if(status > fpsAssigned)
                return 0; // No parsing needed
        }
    }
    m_FilesToBeReparsed.erase(index);
    m_FilesStatus[index]=preliminary ? fpsAssigned : fpsBeingParsed; // Reserve file
    return index;
}

void TokensTree::FlagFileForReparsing(const cc_string& filename)
{
    m_FilesToBeReparsed.insert(GetFileIndex(filename));
}

void TokensTree::FlagFileAsParsed(const cc_string& filename)
{
    m_FilesStatus[GetFileIndex(filename)]=fpsDone;
}
