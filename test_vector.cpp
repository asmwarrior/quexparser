#define _GLIBCXX_BEGIN_NESTED_NAMESPACE(a,b) namespace a {
#define _GLIBCXX_END_NESTED_NAMESPACE }


_GLIBCXX_BEGIN_NESTED_NAMESPACE(std, _GLIBCXX_STD_D)

  /// See bits/stl_deque.h's _Deque_base for an explanation.
  template<typename _Tp, typename _Alloc>
    struct _Vector_base
    {
	;
    };
	

  template<typename _Tp, typename _Alloc>
    inline bool
    operator==(const vector<_Tp, _Alloc>& __x, const vector<_Tp, _Alloc>& __y)
    { return (__x.size() == __y.size()
	      && std::equal(__x.begin(), __x.end(), __y.begin())); }


  template<typename _Tp, typename _Alloc>
    inline bool
    operator<(const vector<_Tp, _Alloc>& __x, const vector<_Tp, _Alloc>& __y)
    { return std::lexicographical_compare(__x.begin(), __x.end(),
					  __y.begin(), __y.end()); }


_GLIBCXX_END_NESTED_NAMESPACE

#endif /* _STL_VECTOR_H */