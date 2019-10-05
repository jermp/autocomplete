Conjunctive top-k
-----------------

Executing queries shuffled at random, for k = 7.

Average among 10 runs.

From the last token of the query, we only retain the first character. This means that we spend less in obtaining the lexicographic range of the character (string comparisons are
very fast), but we spend more on the RMQ phase, because the
range obtained from the completion trie can be very large.

### AOL

#### Solution 1

    {"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "dictionary_search_ns_per_query": "1", "conjunctive_search_ns_per_query": "2718", "reporting_ns_per_query": "328"}
    {"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "537", "dictionary_search_ns_per_query": "4", "conjunctive_search_ns_per_query": "2336", "reporting_ns_per_query": "2270"}
    {"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "1157", "dictionary_search_ns_per_query": "12", "conjunctive_search_ns_per_query": "21008", "reporting_ns_per_query": "1729"}
    {"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1807", "dictionary_search_ns_per_query": "16", "conjunctive_search_ns_per_query": "27837", "reporting_ns_per_query": "944"}
    {"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "2237", "dictionary_search_ns_per_query": "9", "conjunctive_search_ns_per_query": "25749", "reporting_ns_per_query": "531"}
    {"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "2656", "dictionary_search_ns_per_query": "10", "conjunctive_search_ns_per_query": "22094", "reporting_ns_per_query": "412"}
    {"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "3152", "dictionary_search_ns_per_query": "9", "conjunctive_search_ns_per_query": "18857", "reporting_ns_per_query": "429"}
    {"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "4121", "dictionary_search_ns_per_query": "6", "conjunctive_search_ns_per_query": "13913", "reporting_ns_per_query": "789"}


If we do not check the forward index (thus erronously reporting the first k docids of the intersection), we have:

    {"num_terms_per_query": "3", "num_queries": "50000", "conjunctive_search_ns_per_query": "10362"}
    {"num_terms_per_query": "4", "num_queries": "50000", "conjunctive_search_ns_per_query": "21327"}
    {"num_terms_per_query": "5", "num_queries": "50000", "conjunctive_search_ns_per_query": "23187"}
    {"num_terms_per_query": "6", "num_queries": "50000",  "conjunctive_search_ns_per_query": "21259"}
    {"num_terms_per_query": "7", "num_queries": "50000",  "conjunctive_search_ns_per_query": "18234"}
    {"num_terms_per_query": "8+", "num_queries": "50000",  "conjunctive_search_ns_per_query": "13912"}

We can see that the time for the `conjunctive_search` remains the same, except for the case with 3 terms.
This suggests that the time needed to check the forward index is negligible compared to the one
needed to produce the intersection. This can also be observed considering that the time for the case with 2 terms is very small: in this case we check the forward index for each doc in the inverted list of the first term.

#### Solution 2

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "dictionary_search_ns_per_query": "495", "conjunctive_search_ns_per_query": "3082", "reporting_ns_per_query": "632"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "60", "dictionary_search_ns_per_query": "1088", "conjunctive_search_ns_per_query": "592", "reporting_ns_per_query": "1912"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "651", "dictionary_search_ns_per_query": "1063", "conjunctive_search_ns_per_query": "96", "reporting_ns_per_query": "724"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1054", "dictionary_search_ns_per_query": "1085", "conjunctive_search_ns_per_query": "21", "reporting_ns_per_query": "321"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1373", "dictionary_search_ns_per_query": "1115", "conjunctive_search_ns_per_query": "8", "reporting_ns_per_query": "244"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "1911", "dictionary_search_ns_per_query": "1151", "conjunctive_search_ns_per_query": "3", "reporting_ns_per_query": "303"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2260", "dictionary_search_ns_per_query": "1173", "conjunctive_search_ns_per_query": "3", "reporting_ns_per_query": "467"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3222", "dictionary_search_ns_per_query": "1213", "conjunctive_search_ns_per_query": "2", "reporting_ns_per_query": "816"}


### MSN

#### Solution 1

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "2", "dictionary_search_ns_per_query": "3", "conjunctive_search_ns_per_query": "2814", "reporting_ns_per_query": "540"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "438", "dictionary_search_ns_per_query": "5", "conjunctive_search_ns_per_query": "2209", "reporting_ns_per_query": "1872"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "1044", "dictionary_search_ns_per_query": "7", "conjunctive_search_ns_per_query": "12351", "reporting_ns_per_query": "1040"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1558", "dictionary_search_ns_per_query": "8", "conjunctive_search_ns_per_query": "13431", "reporting_ns_per_query": "505"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "2054", "dictionary_search_ns_per_query": "7", "conjunctive_search_ns_per_query": "11448", "reporting_ns_per_query": "295"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "2375", "dictionary_search_ns_per_query": "6", "conjunctive_search_ns_per_query": "9516", "reporting_ns_per_query": "237"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2977", "dictionary_search_ns_per_query": "6", "conjunctive_search_ns_per_query": "8035", "reporting_ns_per_query": "296"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3745", "dictionary_search_ns_per_query": "5", "conjunctive_search_ns_per_query": "6139", "reporting_ns_per_query": "684"}
	
#### Solution 2

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "dictionary_search_ns_per_query": "703", "conjunctive_search_ns_per_query": "3510", "reporting_ns_per_query": "1117"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "32", "dictionary_search_ns_per_query": "1034", "conjunctive_search_ns_per_query": "321", "reporting_ns_per_query": "1370"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "594", "dictionary_search_ns_per_query": "1030", "conjunctive_search_ns_per_query": "51", "reporting_ns_per_query": "459"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1012", "dictionary_search_ns_per_query": "1039", "conjunctive_search_ns_per_query": "10", "reporting_ns_per_query": "208"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1314", "dictionary_search_ns_per_query": "1065", "conjunctive_search_ns_per_query": "4", "reporting_ns_per_query": "170"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "1918", "dictionary_search_ns_per_query": "1077", "conjunctive_search_ns_per_query": "1", "reporting_ns_per_query": "241"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2199", "dictionary_search_ns_per_query": "1098", "conjunctive_search_ns_per_query": "1", "reporting_ns_per_query": "410"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3228", "dictionary_search_ns_per_query": "1126", "conjunctive_search_ns_per_query": "0", "reporting_ns_per_query": "793"}