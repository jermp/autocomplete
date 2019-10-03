Prefix top-k
------------

Executing queries shuffled at random, for k = 7.

Average among 10 runs.

From the last token of the query, we only retain the first character. This means that we spend less in obtaining the lexicographic range of the character (string comparisons are
very fast), but we spend more on the RMQ phase, because the
range obtained from the completion trie can be very large.

### AOL

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "completion_trie_search_ns_per_query": "279", "topk_rmq_ns_per_query": "2887", "reporting_ns_per_query": "317"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "47", "completion_trie_search_ns_per_query": "853", "topk_rmq_ns_per_query": "576", "reporting_ns_per_query": "1851"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "706", "completion_trie_search_ns_per_query": "945", "topk_rmq_ns_per_query": "95", "reporting_ns_per_query": "717"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1055", "completion_trie_search_ns_per_query": "1057", "topk_rmq_ns_per_query": "22", "reporting_ns_per_query": "332"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1495", "completion_trie_search_ns_per_query": "1215", "topk_rmq_ns_per_query": "9", "reporting_ns_per_query": "325"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "1957", "completion_trie_search_ns_per_query": "1434", "topk_rmq_ns_per_query": "3", "reporting_ns_per_query": "425"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2410", "completion_trie_search_ns_per_query": "1581", "topk_rmq_ns_per_query": "2", "reporting_ns_per_query": "611"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3360", "completion_trie_search_ns_per_query": "1888", "topk_rmq_ns_per_query": "2", "reporting_ns_per_query": "913"}

### MSN
    
	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "completion_trie_search_ns_per_query": "403", "topk_rmq_ns_per_query": "3211", "reporting_ns_per_query": "509"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "33", "completion_trie_search_ns_per_query": "784", "topk_rmq_ns_per_query": "312", "reporting_ns_per_query": "1287"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "596", "completion_trie_search_ns_per_query": "906", "topk_rmq_ns_per_query": "49", "reporting_ns_per_query": "423"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1026", "completion_trie_search_ns_per_query": "1015", "topk_rmq_ns_per_query": "11", "reporting_ns_per_query": "206"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1434", "completion_trie_search_ns_per_query": "1114", "topk_rmq_ns_per_query": "5", "reporting_ns_per_query": "217"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "1938", "completion_trie_search_ns_per_query": "1273", "topk_rmq_ns_per_query": "2", "reporting_ns_per_query": "330"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2362", "completion_trie_search_ns_per_query": "1437", "topk_rmq_ns_per_query": "0", "reporting_ns_per_query": "545"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3186", "completion_trie_search_ns_per_query": "1737", "topk_rmq_ns_per_query": "1", "reporting_ns_per_query": "873"}