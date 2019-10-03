- Implement a variant of the solution that eliminates completely the forward index,
  by replacing the completion_trie with a dictionary of the mapped terms using FC.
  We also maintain a mapping from the docID to the lexicographic range of the doc in the
  dictionary.
  During an intersection, for each obtained docID, we decode the corresponding doc
  using the dictionary (we use extract(x), where x = M[docID] is the lex. id of docID).

- Study the effect of compression.

- Implement some competitors: 1. Bast's approach; 2. Explicit union...others?