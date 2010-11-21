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
typedef size_t ItemIdx;

class SearchTreeNode;
class BasicSearchTree;
class SearchTreePoint;


/** SearchTreeLinkMap is the list of the edges towards its child nodes. The character is the
  * key, and the node index is the value
  */
typedef map<char,NodeIdx,less<char> >             SearchTreeLinkMap;

/** SearchTreeNodeArray contains all the nodes for a search tree */
typedef vector<SearchTreeNode*>                   SearchTreeNodeArray;

/** SearchTreePointArray contains a list of tree points defining strings, generally, a tree is just
  * a optimized map<string,int>, this has no restirct that different string(key) could have same value.
  * e.g.
  *      "xxxaaa" -> 1 ;
  *      "xxxbbb" -> 1 ;
  * This is allowed in a general map or trie, but NOT allowed in our basicSearchTree. so we should
  * make our key->value and value->key a bijection, which means different string has different value.
  * This way, e.g. we can give different file names different unique index value.
  * the value is in-fact "item index to the SearchTreePointArray"
  */
typedef vector<SearchTreePoint>                   SearchTreePointArray;

/** SearchTreeItemMap contains all the items belonging to an edge. Each Node has one item map,
  * which
  * is infact a map<depth,itemIdx>, the depth is the string length belong to the current node
  */
typedef map<size_t,size_t,less<size_t> >          SearchTreeItemMap;

/** SearchTreeLabelArray contains all the labels used by the nodes, label can be
  * referenced/accessed by the LabelIdx, which is infact a size_t type
  */
typedef vector<cc_string>                         SearchTreeLabelArray;




/** SearchTreeIterator lets us iterate through the nodes of a BasicSearchTree */
class BasicSearchTreeIterator
{
public:
    /** Default constructor*/
    BasicSearchTreeIterator();
    /** Default constructor, the iterator will point to the root node of the tree*/
    BasicSearchTreeIterator(BasicSearchTree* tree);
    /** Destructor */
    virtual ~BasicSearchTreeIterator() {}

    /** The iterator point to a valid node*/
    bool IsValid();

    /** Move to the previous node
      * @param includeChildren if true, we can move through child node (the last child node)
      */
    bool FindPrev(bool includeChildren = true);

    /** Move to the next node
      * @param includeChildren if true, we can move through child node (the first child node)
      */
    bool FindNext(bool includeChildren = true);

    /** operator * overloading function, return the NodeIdx (size_t) in the Node Vector*/
    const NodeIdx& operator* () const
    {
        return m_NodeIdx;
    }

    /** Move to the next Node, include Child node*/
    const BasicSearchTreeIterator& operator++()
    {
        FindNext();
        return *this;
    }
    /** Move backward, include child node*/
    const BasicSearchTreeIterator& operator--()
    {
        FindPrev();
        return *this;
    }

    /** Go to the next Sibling node
      * @return if true, the move is succefull
      */
    bool FindNextSibling();

    /** Go to the previous Sibling node
      * @return if true, the move is succefull
      */
    bool FindPrevSibling();

    /** currently, this function is not used any more*/
    bool FindSibling(char ch);

    /** Check the Iterator is reached the Eof*/
    bool Eof()
    {
        return (!IsValid() || m_Eof);
    }

    /** Node Index, size_t */
    NodeIdx                     m_NodeIdx;
    /** Reached end of tree */
    bool                        m_Eof;

protected:

    /** basic search tree pointer */
    BasicSearchTree            *m_pTree;

    /** For checking validity */
    size_t                      m_LastTreeSize;

    /** For checking validity */
    SearchTreeNode             *m_LastAddedNode;

};
/** locate a point (string) element in the tree
  * the NodeIdx n is to reference the node
  * the depth is oused as the key in the itmes map of the node
  * once the above two variable is specified, the associated string can be caculated
  */
class SearchTreePoint
{
public:

    /** default constructor pointer to the root node*/
    SearchTreePoint ():n(0),depth(0) {}

    /** constructor pointing to a sepcified string element */
    SearchTreePoint (NodeIdx nn, size_t dd)
    {
        n = nn;
        depth = dd;
    }

    /** Which node are we pointing to */
    NodeIdx n;

    /** At what depth is the string's end located?*/
    size_t depth;
};

/** Node of the tree, it contains a edge(label), a link map(linking to its child node, a item map( a
  * map<string depth,Node index>.
  */
class SearchTreeNode
{
    friend class BasicSearchTree;
    friend class BasicSearchTreeIterator;
public:
    /** defualt constructor*/
    SearchTreeNode();

    /** \brief constructor
      *
      * @param depth the Node's depth is the depth of the first char of the label(from the root to the char)
      * @param parent parent node index in the node vector
      * @param label  label index in the label vector
      * @param labelstart label start position
      * @param labellen   label end position
      *
      */
    SearchTreeNode(unsigned int depth,NodeIdx parent,LabelIdx label, unsigned int labelstart, unsigned int labellen);

    /** destructor*/
    virtual ~SearchTreeNode();

    /** return the parent node index (size_t) */
    NodeIdx GetParent() const
    {
        return m_Parent;
    }

    /** return the parent node index (size_t) */
    void SetParent(NodeIdx newparent)
    {
        m_Parent = newparent;
    }
    NodeIdx GetChild(char ch);


    /** return the associatied item index ( item number, or size_t type)
      *
      * @param depth input variable, this is the search key in the item map
      * @return the item index
      * operations on the item map, which is a map (depth->number)
      */
    ItemIdx GetItemIdx(size_t depth);


    /** try to add a pair (key=depth, value=item index)
      *
      * @param depth input search key on the item map
      * @param  itemIdx input item. if the value associated with the depth is empty, just set the itemIdx
      * @return the number associated with the input depth
      * operations on the item map, which is a map (depth->number)
      * node: if the item is alreay exist in the item map, then just return the index, thus the
      * input itemIdx is discarded
      */
    ItemIdx AddItemIdx(size_t depth, ItemIdx itemIdx);


    /** get the parent Node address
      *
      * @param tree input searchTree address
      * @return parent SearchTreeNode address
      */
    SearchTreeNode* GetParent(const BasicSearchTree* tree) const;

    /** get the child Node address with the label(the first character was ch)
      *
      * @param tree BasicSearchTree pointer
      * @param ch link map key
      * @return SearchTreeNode*
      * operation on the link map, which is a map(key=char, value=NodeIdx)
      */
    SearchTreeNode* GetChild(BasicSearchTree* tree,char ch);

    /**  Get the Label string of the Node
      * @param tree a pointer to BasicSearchTree
      * @return  label string
      */
    cc_string GetLabel(const BasicSearchTree* tree) const;

    /**  get the first character of the Node
      * @param tree input BasicSearchTree pointer
      * @return first char
      */
    char GetChar(const BasicSearchTree* tree) const;

    /** get the total reference label string. no start and length is conserned.
      * @param tree const BasicSearchTree*
      * @return const cc_string&
      */
    const cc_string& GetActualLabel(const BasicSearchTree* tree) const;

    /// read and set the member variables
    LabelIdx GetLabelIdx() const
    {
        return m_Label;
    }
    /** read the label start position. the first char position */
    unsigned int GetLabelStart() const
    {
        return m_LabelStart;
    }
    /** the length of the edge*/
    unsigned int GetLabelLen() const
    {
        return m_LabelLen;
    }
    /** set the label string
      * @param label label index
      * @param labelStart label start position(first char index on the label string)
      * @param labelLen edge length
      */
    void SetLabel(LabelIdx label, unsigned int labelstart, unsigned int labellen);

    /** Node depth, it is the depth of the first char of the label (edge) */
    unsigned int GetDepth() const
    {
        return m_Depth;
    }

    /** this always happends when the new node or the new branch is added
      * Updates the depth of the tree
      */
    void RecalcDepth(BasicSearchTree* tree);

    /** Updates items with parent
      *
      */
    void UpdateItems(BasicSearchTree* tree);

    /**  Returns the depth of the start of the node's incoming label
      *  which is the first character of the label.
      *  it is just m_Depth - m_LabelLen
      *  In other words, returns the (calculated) parent's depth
      */
    unsigned int GetLabelStartDepth() const;

    /** The label's depth is 0-based (from the root) */
    bool IsLeaf() const
    {
        return m_Children.empty() && (m_Depth != 0);
    }

    /** Gets the deepest position where the string matches the node's edge's label.
      *  0 for 0 characters in the tree matched, 1 for 1 character matched, etc.
      */
    unsigned int GetDeepestMatchingPosition(BasicSearchTree* tree, const cc_string& s,unsigned int StringStartDepth);
    cc_string Serialize(BasicSearchTree* tree,NodeIdx node_id,bool withchildren = false);
    void dump(BasicSearchTree* tree,NodeIdx node_id,const cc_string& prefix,cc_string& result);


    /** helper function, xml format output */
    static cc_string SerializeString(const cc_string& s);

    /** unsigned int to string */
    static cc_string u2s(unsigned int u);

    /** int to string */
    static cc_string i2s(int i);

    /** xml input*/
    static bool UnSerializeString(const cc_string& s,cc_string& result);

    /** string to unsign int*/
    static bool s2u(const cc_string& s,unsigned int& u);

    /** string to int*/
    static bool s2i(const cc_string& s,int& i);
protected:
    unsigned int          m_Depth;
    NodeIdx               m_Parent;
    LabelIdx              m_Label;
    unsigned int          m_LabelStart, m_LabelLen;
    SearchTreeLinkMap     m_Children;
    SearchTreeItemMap     m_Items;
};
/** a map<string,size_t>, different string should have different value, so this is also
  * a map<size_t,string>. a bi-direction map.
  * it is the base of the Tokenstree.
  * the tree has a nodeArray, a labelArray and a pointArray, all these element was accessed by index
  * the point index is sometimes called item number (itemno) or item index.
  */
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

    /** Gets the number of items stored */
    virtual size_t GetCount() const
    {
        return m_Points.size();
    }
    /** clears items and tree and point array */
    virtual void clear();

    /** Adds an item number to position defined by s.
      * If the string already exists, returns the correspoinding item number.
      */
    ItemIdx insert(const cc_string& s);

    /** Tells if there is an item for string s */
    bool HasItem(const cc_string& s);

    /** std::map compatibility for the above */
    size_t count(const cc_string& s)
    {
        return HasItem(s) ? 1 : 0;
    }

    /** Gets the point array position defined by s */
    ItemIdx GetItemIdx(const cc_string& s);

    /** Gets the key string for item index */
    const cc_string GetString(ItemIdx itemIdx) const;

    /** Finds items that match a given string.
      *  if is_prefix==true, it finds items that start with the string.
      *  returns the number of matches.
      */
    size_t FindMatches(const cc_string& s,set<size_t> &result,bool caseSensitive,bool prefixMatch);

    /** Serializes the labels into an XML-compatible string */
    cc_string SerializeLabels();

    /** Dumps a graphical version of the tree */
    cc_string dump();

protected:

    /** Creates a new node. Function is virtual so the nodes can be extended
      * and customized, or to improve the memory management.
      */
    virtual SearchTreeNode* CreateNode(unsigned int depth,
                                       NodeIdx parent,
                                       LabelIdx label,
                                       unsigned int labelstart,
                                       unsigned int labellen);

    /** Gets the string corresponding to the tree 'point'.
      * If topNode is specified, it gets the string that goes from node 'topNode' to 'point'.
      */
    cc_string GetString(const SearchTreePoint &point,NodeIdx topNode = 0) const;

    /** Obtains the node with number n,NULL if n is invalid.
      * If NullOnZero == true, returns NULL if n is 0.
      */
    SearchTreeNode* GetNode(NodeIdx n,bool NullOnZero = false);

    /** Finds the node that starts from node 'parentIdx', and has the suffix s. */
    bool FindNode(const cc_string& s, NodeIdx parentIdx, SearchTreePoint* result);

    /** Adds Suffix s starting from node parentIdx.*/
    SearchTreePoint AddNode(const cc_string& s, NodeIdx parentIdx = 0);

    /** Serializes given label into an XML-escaped string.*/
    cc_string SerializeLabel(LabelIdx labelno);





    /** Labels Array used by the nodes' edges */
    SearchTreeLabelArray m_Labels;

    /** Nodes Array and their edges */
    SearchTreeNodeArray m_pNodes;

    /** Points Array defining the items' strings */
    SearchTreePointArray m_Points;

private:
    /** Creates the tree's root node.*/
    void CreateRootNode();

    /** Splits the Branch that leads to node n, at the given depth.
      * Used by AddNode.
      * @return the newly created node
      * if the given position is exactly the length of n's vertex,
      * just return n.
      */
    NodeIdx SplitBranch(NodeIdx n,size_t depth);
};



/** template class, adding a array of T in the SearchTree class*/
template <class T>
class SearchTree: public BasicSearchTree
{
public:
    SearchTree();
    virtual ~SearchTree();

    /** clears the tree */
    virtual void clear();

    /** Gets the number of items stored */
    size_t GetCount() const;

    /** Same as GetCount*/
    virtual size_t size() const;

    /** Stores the Tree and items into a file*/
    bool SaveCacheTo(const cc_string& filename);

    /** Loads the Tree and items from a file */
    bool LoadCacheFrom(const cc_string& filename);

    /** XML output of the tree */
    cc_string Serialize();

    /** Gets the item at position defined by string s */
    T GetItem(const cc_string& s);

    /** Gets the item at position defined by char * s */
    T GetItem(const char* s);

    /** Adds an item to position defined by s*/
    size_t AddItem(const cc_string& s,T item,bool replaceexisting = false);

    /** Gets the item found at position i */
    T& GetItemAtPos(size_t i);

    /** Replaces the item found at position i */
    void SetItemAtPos(size_t i,T item);

    /** Gets the item found at position s. Inserts new empty one if not found. */
    T& operator[](const cc_string& s);

    /** Serializes the stored items */
    virtual cc_string SerializeItem(size_t idx)
    {
        return cc_text("");
    }
    /** Unserializes the items to be stored */
    virtual void* UnserializeItem(const cc_string& s)
    {
        return NULL;
    }
protected:
    /**  The actual stored items Array*/
    vector<T> m_Items;

    /** Releases the stored items from memory. Called by clear();*/
    virtual void clearItems();

    /** Adds a null item to position 0.*/
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
    size_t itemno = GetItemIdx(s);
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
    size_t curpos = GetItemIdx(s);
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
