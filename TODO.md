- Implement the search of a token that is not complete Q[m] in the completion_trie:
    1. obtain the lexicographic range [l,r] of Q[m]
    2. search Q[1,m-1] in the completion_trie to arrive at the level whose nodes correspond
       to terms that are prefixed by Q[m]. Locate the range [l,r] in the level and report
       the corresponding range [p,q]
    3. proceed with RMQ in the range [p,q]

- Implement a variant of the solution that eliminates completely the forward index,
  by replacing the completion_trie with a dictionary of the mapped terms using FC.
  We also maintain a mapping from the docID to the lexicographic range of the doc in the
  dictionary.
  During an intersection, for each obtained docID, we decode the corresponding doc
  using the dictionary (we use extract(x), where x = M[docID] is the lex. id of docID).

- Study the effect of compression.