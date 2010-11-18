/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */
#ifndef SEARCHTREE_H
#define SEARCHTREE_H

#include <vector>
#include <map>
#include <set>

#include "cchelper.h"

using namespace std;

typedef size_t NodeIdx;
typedef size_t LabelIdx;

class SearchTreeNode;
class BasicSearchTree;
class SearchTreePoint;


/** SearchTreeLinkMap is the list of the edges towards other nodes. The character is the
key, and the node is the value */
typedef map<char,NodeIdx,less<char> >             SearchTreeLinkMap;

typedef vector<SearchTreeLinkMap::iterator>       SearchTreeStack;

/** SearchTreeNodeArray contains all the nodes for a search tree */
typedef vector<SearchTreeNode*>                   SearchTreeNodeArray;

/** SearchTreePointArray contains a list of tree points defining strings */
typedef vector<SearchTreePoint>                   SearchTreePointArray;

/** SearchTreeItemMap contains all the items belonging to an edge */
typedef map<size_t,size_t,less<size_t> >          SearchTreeItemMap;

/** SearchTreeLabelArray contains the labels used by the nodes */
typedef vector<cc_string>                         SearchTreeLabelArray;




/** SearchTreeIterator lets us iterate through the nodes of a BasicSearchTree */
class BasicSearchTreeIterator
{
public:
    BasicSearchTreeIterator();
    BasicSearchTreeIterator(BasicSearchTree* tree);
    virtual ~BasicSearchTreeIterator() {}
    bool IsValid();
    bool FindPrev(bool includechildren = true);
    bool FindNext(bool includechildren = true);
    const NodeIdx& operator* () const
    {
        return m_CurNode;
    }
    const BasicSearchTreeIterator& operator++()
    {
        FindNext();
        return *this;
    }
    const BasicSearchTreeIterator& operator--()
    {
        FindPrev();
        return *this;
    }
    bool FindNextSibling();
    bool FindPrevSibling();
    bool FindSibling(char ch);
    bool eof()
    {
        return (!IsValid() || m_eof);
    }
    NodeIdx m_CurNode;
    bool m_eof; // Reached end of tree

protected:
    BasicSearchTree            *m_pTree;
    size_t                      m_LastTreeSize;    // For checking validity
    SearchTreeNode             *m_LastAddedNode;   // For checking validity
    SearchTreeStack             m_Stack;
    vector<SearchTreeLinkMap*>  m_Stack2;
};

class SearchTreePoint
{
public:
    NodeIdx n;    /// Which node are we pointing to?
    size_t depth; /// At what depth is the string's end located?

    SearchTreePoint ():n(0),depth(0) {}

    SearchTreePoint (NodeIdx nn, size_t dd)
    {
        n = nn;
        depth = dd;
    }
};

class SearchTreeNode
{
    friend class BasicSearchTree;
    friend class BasicSearchTreeIterator;
public:
    SearchTreeNode();
    SearchTreeNode(unsigned int depth,NodeIdx parent,LabelIdx label, unsigned int labelstart, unsigned int labellen);
    virtual ~SearchTreeNode();
    NodeIdx GetParent() const
    {
        return m_parent;
    }
    void SetParent(NodeIdx newparent)
    {
        m_parent = newparent;
    }
    NodeIdx GetChild(char ch);


    /** \brief return the associatied item (a number)
     *
     * \param depth
     * \return number
     * operations on the item map, which is a map (depth->number)
     */
    size_t GetItemNo(size_t depth);


    /** \brief try to add a pair (key=depth, value=itemno)
     *
     * \param depth
     * \param itemno
     * \return the number associated with the input depth
     * operations on the item map, which is a map (depth->number)
     */
    size_t AddItemNo(size_t depth,size_t itemno);


    /** \brief get the parent Node address
     *
     * \param tree input searchTree address
     * \return parent SearchTreeNode address
     *
     */
    SearchTreeNode* GetParent(const BasicSearchTree* tree) const;

    /** \brief get the child Node address with the label(the first character was ch)
     *
     * \param tree BasicSearchTree pointer
     * \param ch link map key
     * \return SearchTreeNode*
     * operation on the link map, which is a map(key=char, value=NodeIdx)
     */
    SearchTreeNode* GetChild(BasicSearchTree* tree,char ch);

    /** \brief Get the Label string of the Node
     *
     * \param tree a pointer to BasicSearchTree
     * \return  label string
     *
     */
    cc_string GetLabel(const BasicSearchTree* tree) const;

    /** \brief get the first character of the Node
     *
     * \param tree input BasicSearchTree pointer
     * \return first char
     *
     */
    char GetChar(const BasicSearchTree* tree) const;

    /** \brief get the total reference label string. no start and length is conserned.
     *
     * \param tree const BasicSearchTree*
     * \return const cc_string&
     *
     */
    const cc_string& GetActualLabel(const BasicSearchTree* tree) const;

    /// read and set the member variables
    LabelIdx GetLabelNo() const
    {
        return m_label;
    }

    unsigned int GetLabelStart() const
    {
        return m_labelstart;
    }

    unsigned int GetLabelLen() const
    {
        return m_labellen;
    }

    void SetLabel(LabelIdx label, unsigned int labelstart, unsigned int labellen);

    unsigned int GetDepth() const
    {
        return m_depth;
    }

    void RecalcDepth(BasicSearchTree* tree); /// Updates the depth
    void UpdateItems(BasicSearchTree* tree); /// Updates items with parent

    /** Returns the depth of the start of the node's incoming label
        which is the first character of the label.
        it is just m_depth - m_labellen
        In other words, returns the (calculated) parent's depth */
    unsigned int GetLabelStartDepth() const;

    /// The label's depth is 0-based.
    bool IsLeaf() const
    {
        return m_Children.empty() && (m_depth != 0);
    }

    /** Gets the deepest position where the string matches the node's edge's label.
        0 for 0 characters in the tree matched, 1 for 1 character matched, etc.
        */
    unsigned int GetDeepestMatchingPosition(BasicSearchTree* tree, const cc_string& s,unsigned int StringStartDepth);
    cc_string Serialize(BasicSearchTree* tree,NodeIdx node_id,bool withchildren = false);
    void dump(BasicSearchTree* tree,NodeIdx node_id,const cc_string& prefix,cc_string& result);


    static cc_string SerializeString(const cc_string& s);
    static cc_string u2s(unsigned int u);
    static cc_string i2s(int i);
    static bool UnSerializeString(const cc_string& s,cc_string& result);
    static bool s2u(const cc_string& s,unsigned int& u);
    static bool s2i(const cc_string& s,int& i);
protected:
    unsigned int          m_depth;
    NodeIdx               m_parent;
    LabelIdx              m_label;
    unsigned int          m_labelstart, m_labellen;
    SearchTreeLinkMap     m_Children;
    SearchTreeItemMap     m_Items;
};

class BasicSearchTree
{
    friend class SearchTreeNode;
    friend class BasicSearchTreeIterator;
public:
    BasicSearchTree();
    virtual ~BasicSearchTree();
    virtual size_t size() const
    {
        return m_Points.size();
    }
    virtual size_t GetCount() const
    {
        return m_Points.size();    /// Gets the number of items stored
    }
    virtual void clear(); /// clears items and tree

    /** Adds an item number to position defined by s.
        If the string already exists, returns the correspoinding item no. */
    size_t insert(const cc_string& s);

    /// Tells if there is an item for string s
    bool HasItem(const cc_string& s);

    /// std::map compatibility for the above
    size_t count(const cc_string& s)
    {
        return HasItem(s) ? 1 : 0;
    }

    /// Gets the array position defined by s
    size_t GetItemNo(const cc_string& s);

    /// Gets the key string for item n
    const cc_string GetString(size_t pointIdx) const;

    /** Finds items that match a given string.
        if is_prefix==true, it finds items that start with the string.
        returns the number of matches.
    */
    size_t FindMatches(const cc_string& s,set<size_t> &result,bool caseSensitive,bool is_prefix);

    /// Serializes the labels into an XML-compatible string
    cc_string SerializeLabels();
    /// Dumps a graphical version of the tree
    cc_string dump();
protected:

    /** Creates a new node. Function is virtual so the nodes can be extended
        and customized, or to improve the memory management. */
    virtual SearchTreeNode* CreateNode(unsigned int depth,NodeIdx parent,LabelIdx label, unsigned int labelstart, unsigned int labellen);

    /** Gets the string corresponding to the tree point 'nn'.
        If 'top' is specified, it gets the string that goes from node 'top' to point 'nn'. */
    cc_string GetString(const SearchTreePoint &point,NodeIdx topNode = 0) const;

    /** Obtains the node with number n,NULL if n is invalid.
        If NullOnZero == true, returns NULL if n is 0. */
    SearchTreeNode* GetNode(NodeIdx n,bool NullOnZero = false);
    /// Finds the node that starts from node 'parent', and has the suffix s.
    bool FindNode(const cc_string& s, NodeIdx nparent, SearchTreePoint* result);
    /// Adds Suffix s starting from node nparent.
    SearchTreePoint AddNode(const cc_string& s, NodeIdx nparent = 0);

    /// Serializes given label into an XML-escaped string.
    cc_string SerializeLabel(LabelIdx labelno);

    /// Labels used by the nodes' edges
    SearchTreeLabelArray m_Labels;
    /// Nodes and their edges
    SearchTreeNodeArray m_pNodes;

    /// Points defining the items' strings
    SearchTreePointArray m_Points;

private:
    /// Creates the tree's root node.
    void CreateRootNode();

    /** Splits the Branch that leads to node n, at the given depth.
        Used by AddNode.
        @return the newly created node
        if the given position is exactly the length of n's vertex,
        just return n.
     */
    NodeIdx SplitBranch(NodeIdx n,size_t depth);
};

template <class T>
class SearchTree: public BasicSearchTree
{
public:
    SearchTree();
    virtual ~SearchTree();
    virtual void clear(); /// clears the tree
    size_t GetCount() const; /// Gets the number of items stored
    virtual size_t size() const; /// Same as GetCount
    bool SaveCacheTo(const cc_string& filename); /// Stores the Tree and items into a file
    bool LoadCacheFrom(const cc_string& filename); /// Loads the Tree and items from a file
    cc_string Serialize();
    T GetItem(const cc_string& s); /// Gets the item at position defined by s
    T GetItem(const char* s);
    size_t AddItem(const cc_string& s,T item,bool replaceexisting = false); /// Adds an item to position defined by s
    T& GetItemAtPos(size_t i); /// Gets the item found at position i
    void SetItemAtPos(size_t i,T item); /// Replaces the item found at position i

    /** Gets the item found at position s. Inserts new empty one if not found. */
    T& operator[](const cc_string& s);
    /// Serializes the stored items
    virtual cc_string SerializeItem(size_t idx)
    {
        return cc_text("");
    }
    /// Unserializes the items to be stored
    virtual void* UnserializeItem(const cc_string& s)
    {
        return NULL;
    }
protected:
    vector<T> m_Items;   /// The actual stored items

    /// Releases the stored items from memory. Called by clear();
    virtual void clearItems();

    /// Adds a null item to position 0.
    virtual bool AddFirstNullItem();

};

template <class T>
SearchTree<T>::SearchTree():BasicSearchTree()
{
    m_Items.clear();
    AddFirstNullItem();
}

template <class T>
SearchTree<T>::~SearchTree()
{
    clearItems();
}

template <class T>
void SearchTree<T>::clear()
{
    clearItems();
    BasicSearchTree::clear();
    AddFirstNullItem();
}

template <class T>
size_t SearchTree<T>::GetCount() const
{
    size_t result = m_Items.size() -1;
    return result;
}

template <class T>
size_t SearchTree<T>::size() const
{
    size_t result = m_Items.size() -1;
    return result;
}


template <class T>
bool SearchTree<T>::SaveCacheTo(const cc_string& filename)
{
    return true;
}

template <class T>
bool SearchTree<T>::LoadCacheFrom(const cc_string& filename)
{
    return true;
}

template <class T>
T SearchTree<T>::GetItem(const char* s)
{
    cc_string tmps(s);
    return GetItem(tmps);
}

template <class T>
T SearchTree<T>::GetItem(const cc_string& s)
{
    size_t itemno = GetItemNo(s);
    if(!itemno && !s.empty())
        return T();
    return GetItemAtPos(itemno);
}

template <class T>
size_t SearchTree<T>::AddItem(const cc_string& s,T item,bool replaceexisting)
{
    size_t itemno = insert(s);
    if(itemno > m_Items.size())
        m_Items.resize(itemno);
    else if(itemno == m_Items.size())
        m_Items.push_back(item);
    else if(replaceexisting)
        m_Items[itemno] = item;
    return itemno;
}

template <class T>
T& SearchTree<T>::GetItemAtPos(size_t i)
{
    if(i>=m_Items.size() || i < 1)
        i = 0;
    return m_Items[i];
}

template <class T>
void SearchTree<T>::SetItemAtPos(size_t i,T item)
{
    m_Items[i]=item;
}

template <class T>
void SearchTree<T>::clearItems()
/// Called by BasicSearchTree::BasicSearchTree() and BasicSearchTree::clear()
{
    m_Items.clear();
}

template <class T>
bool SearchTree<T>::AddFirstNullItem()
{
    T newvalue;
    m_Items.push_back(newvalue);
    return true;
}

template <class T>
T& SearchTree<T>::operator[](const cc_string& s)
{
    size_t curpos = GetItemNo(s);
    if(!curpos)
    {
        T newitem;
        curpos = AddItem(s,newitem);
    }
    return m_Items[curpos];
}

template <class T>
cc_string SearchTree<T>::Serialize()
{
    cc_string result;
    size_t i;
    result << "<SearchTree>\n";
    result << SerializeLabels();
    result << "<nodes>\n";
    for(i=0; i<m_pNodes.size(); i++)
        result << m_pNodes[i]->Serialize(this,i,false);
    result << "</nodes>\n";
    result << " <items>\n";
    for(i=1; i < m_Items.size(); i++)
    {
        result << SerializeItem(i);

    }
    result << " </items>\n";
    result << "</SearchTree>\n";
    return result;
}

#endif
