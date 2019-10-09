Prefix top-k
------------

Executing queries shuffled at random, for k = 7.

Average among 10 runs.

From the last token of the query, we only retain the first character. This means that we spend less in obtaining the lexicographic range of the character (string comparisons are
very fast), but we spend more on the RMQ phase, because the
range obtained from the completion trie can be very large.

### AOL

#### Solution 1

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "completions_search_ns_per_query": "279", "topk_rmq_ns_per_query": "2887", "reporting_ns_per_query": "317"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "47", "completions_search_ns_per_query": "853", "topk_rmq_ns_per_query": "576", "reporting_ns_per_query": "1851"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "706", "completions_search_ns_per_query": "945", "topk_rmq_ns_per_query": "95", "reporting_ns_per_query": "717"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1055", "completions_search_ns_per_query": "1057", "topk_rmq_ns_per_query": "22", "reporting_ns_per_query": "332"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1495", "completions_search_ns_per_query": "1215", "topk_rmq_ns_per_query": "9", "reporting_ns_per_query": "325"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "1957", "completions_search_ns_per_query": "1434", "topk_rmq_ns_per_query": "3", "reporting_ns_per_query": "425"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2410", "completions_search_ns_per_query": "1581", "topk_rmq_ns_per_query": "2", "reporting_ns_per_query": "611"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3360", "completions_search_ns_per_query": "1888", "topk_rmq_ns_per_query": "2", "reporting_ns_per_query": "913"}

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "5027"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "4974"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "3984"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "4137"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "4660"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "5335"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "5785"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "7394"}

#### Solution 2

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "completions_search_ns_per_query": "493", "topk_rmq_ns_per_query": "3072", "reporting_ns_per_query": "628"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "60", "completions_search_ns_per_query": "1078", "topk_rmq_ns_per_query": "589", "reporting_ns_per_query": "1897"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "675", "completions_search_ns_per_query": "1053", "topk_rmq_ns_per_query": "96", "reporting_ns_per_query": "730"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1047", "completions_search_ns_per_query": "1081", "topk_rmq_ns_per_query": "21", "reporting_ns_per_query": "320"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1367", "completions_search_ns_per_query": "1112", "topk_rmq_ns_per_query": "8", "reporting_ns_per_query": "244"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "1886", "completions_search_ns_per_query": "1139", "topk_rmq_ns_per_query": "3", "reporting_ns_per_query": "300"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2242", "completions_search_ns_per_query": "1166", "topk_rmq_ns_per_query": "3", "reporting_ns_per_query": "455"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3229", "completions_search_ns_per_query": "1205", "topk_rmq_ns_per_query": "2", "reporting_ns_per_query": "809"}

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "5768"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "5625"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "4389"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "4421"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "4830"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "5336"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "5963"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "7104"}

### MSN
    
#### Solution 1
    
	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "completions_search_ns_per_query": "403", "topk_rmq_ns_per_query": "3211", "reporting_ns_per_query": "509"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "33", "completions_search_ns_per_query": "784", "topk_rmq_ns_per_query": "312", "reporting_ns_per_query": "1287"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "596", "completions_search_ns_per_query": "906", "topk_rmq_ns_per_query": "49", "reporting_ns_per_query": "423"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1026", "completions_search_ns_per_query": "1015", "topk_rmq_ns_per_query": "11", "reporting_ns_per_query": "206"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1434", "completions_search_ns_per_query": "1114", "topk_rmq_ns_per_query": "5", "reporting_ns_per_query": "217"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "1938", "completions_search_ns_per_query": "1273", "topk_rmq_ns_per_query": "2", "reporting_ns_per_query": "330"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2362", "completions_search_ns_per_query": "1437", "topk_rmq_ns_per_query": "0", "reporting_ns_per_query": "545"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3186", "completions_search_ns_per_query": "1737", "topk_rmq_ns_per_query": "1", "reporting_ns_per_query": "873"}
	
	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "5804"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "4006"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "3456"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "3873"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "4587"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "5030"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "5617"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "6957"}

#### Solution 2

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "completions_search_ns_per_query": "697", "topk_rmq_ns_per_query": "3495", "reporting_ns_per_query": "1114"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "32", "completions_search_ns_per_query": "1038", "topk_rmq_ns_per_query": "321", "reporting_ns_per_query": "1384"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "547", "completions_search_ns_per_query": "1029", "topk_rmq_ns_per_query": "51", "reporting_ns_per_query": "455"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1012", "completions_search_ns_per_query": "1038", "topk_rmq_ns_per_query": "11", "reporting_ns_per_query": "210"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1318", "completions_search_ns_per_query": "1066", "topk_rmq_ns_per_query": "5", "reporting_ns_per_query": "172"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "1922", "completions_search_ns_per_query": "1077", "topk_rmq_ns_per_query": "1", "reporting_ns_per_query": "242"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2213", "completions_search_ns_per_query": "1099", "topk_rmq_ns_per_query": "1", "reporting_ns_per_query": "425"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3228", "completions_search_ns_per_query": "1124", "topk_rmq_ns_per_query": "0", "reporting_ns_per_query": "799"}
	
	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "6772"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "4646"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "3831"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "4108"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "4594"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "5080"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "5621"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "6775"}