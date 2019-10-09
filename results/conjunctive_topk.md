Conjunctive top-k
-----------------

Executing queries shuffled at random, for k = 7.

Average among 10 runs.

From the last token of the query, we only retain the first character. This means that we spend less in obtaining the lexicographic range of the character (string comparisons are
very fast), but we spend more on the RMQ phase, because the
range obtained from the completion trie can be very large.

### AOL

#### Solution 1

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "dictionary_search_ns_per_query": "3", "conjunctive_search_ns_per_query": "2896", "reporting_ns_per_query": "352"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "52", "dictionary_search_ns_per_query": "10", "conjunctive_search_ns_per_query": "2273", "reporting_ns_per_query": "2333"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "919", "dictionary_search_ns_per_query": "39", "conjunctive_search_ns_per_query": "20478", "reporting_ns_per_query": "1772"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1298", "dictionary_search_ns_per_query": "49", "conjunctive_search_ns_per_query": "27363", "reporting_ns_per_query": "974"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1857", "dictionary_search_ns_per_query": "42", "conjunctive_search_ns_per_query": "25484", "reporting_ns_per_query": "556"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "2239", "dictionary_search_ns_per_query": "34", "conjunctive_search_ns_per_query": "22070", "reporting_ns_per_query": "438"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2871", "dictionary_search_ns_per_query": "32", "conjunctive_search_ns_per_query": "18657", "reporting_ns_per_query": "465"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3774", "dictionary_search_ns_per_query": "30", "conjunctive_search_ns_per_query": "13967", "reporting_ns_per_query": "844"}

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "4463"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "6677"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "25503"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "31536"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "29973"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "27148"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "23630"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "20511"}

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

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "dictionary_search_ns_per_query": "6", "conjunctive_search_ns_per_query": "3275", "reporting_ns_per_query": "330"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "109", "dictionary_search_ns_per_query": "36", "conjunctive_search_ns_per_query": "15770", "reporting_ns_per_query": "2485"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "932", "dictionary_search_ns_per_query": "52", "conjunctive_search_ns_per_query": "24290", "reporting_ns_per_query": "1780"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1388", "dictionary_search_ns_per_query": "55", "conjunctive_search_ns_per_query": "29056", "reporting_ns_per_query": "953"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1880", "dictionary_search_ns_per_query": "41", "conjunctive_search_ns_per_query": "26675", "reporting_ns_per_query": "541"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "2277", "dictionary_search_ns_per_query": "43", "conjunctive_search_ns_per_query": "22955", "reporting_ns_per_query": "421"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2762", "dictionary_search_ns_per_query": "37", "conjunctive_search_ns_per_query": "19437", "reporting_ns_per_query": "443"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3878", "dictionary_search_ns_per_query": "40", "conjunctive_search_ns_per_query": "14657", "reporting_ns_per_query": "814"}

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "4917"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "20361"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "28619"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "33140"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "30410"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "27477"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "24357"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "21042"}

### MSN

#### Solution 1

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "dictionary_search_ns_per_query": "6", "conjunctive_search_ns_per_query": "3021", "reporting_ns_per_query": "576"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "39", "dictionary_search_ns_per_query": "7", "conjunctive_search_ns_per_query": "2279", "reporting_ns_per_query": "1926"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "810", "dictionary_search_ns_per_query": "15", "conjunctive_search_ns_per_query": "12382", "reporting_ns_per_query": "1078"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1104", "dictionary_search_ns_per_query": "15", "conjunctive_search_ns_per_query": "13534", "reporting_ns_per_query": "526"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1737", "dictionary_search_ns_per_query": "11", "conjunctive_search_ns_per_query": "11424", "reporting_ns_per_query": "305"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "2049", "dictionary_search_ns_per_query": "10", "conjunctive_search_ns_per_query": "9565", "reporting_ns_per_query": "252"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2396", "dictionary_search_ns_per_query": "9", "conjunctive_search_ns_per_query": "8020", "reporting_ns_per_query": "324"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3431", "dictionary_search_ns_per_query": "9", "conjunctive_search_ns_per_query": "6199", "reporting_ns_per_query": "738"}

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "4982"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "6176"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "16236"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "17306"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "15591"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "13961"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "12980"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "12311"}

#### Solution 2

	{"num_terms_per_query": "1", "num_queries": "50000", "parsing_ns_per_query": "0", "dictionary_search_ns_per_query": "6", "conjunctive_search_ns_per_query": "3722", "reporting_ns_per_query": "511"}
	{"num_terms_per_query": "2", "num_queries": "50000", "parsing_ns_per_query": "56", "dictionary_search_ns_per_query": "20", "conjunctive_search_ns_per_query": "15134", "reporting_ns_per_query": "2043"}
	{"num_terms_per_query": "3", "num_queries": "50000", "parsing_ns_per_query": "835", "dictionary_search_ns_per_query": "20", "conjunctive_search_ns_per_query": "15310", "reporting_ns_per_query": "1072"}
	{"num_terms_per_query": "4", "num_queries": "50000", "parsing_ns_per_query": "1117", "dictionary_search_ns_per_query": "19", "conjunctive_search_ns_per_query": "14672", "reporting_ns_per_query": "517"}
	{"num_terms_per_query": "5", "num_queries": "50000", "parsing_ns_per_query": "1704", "dictionary_search_ns_per_query": "14", "conjunctive_search_ns_per_query": "12384", "reporting_ns_per_query": "300"}
	{"num_terms_per_query": "6", "num_queries": "50000", "parsing_ns_per_query": "2164", "dictionary_search_ns_per_query": "13", "conjunctive_search_ns_per_query": "10222", "reporting_ns_per_query": "246"}
	{"num_terms_per_query": "7", "num_queries": "50000", "parsing_ns_per_query": "2567", "dictionary_search_ns_per_query": "12", "conjunctive_search_ns_per_query": "8579", "reporting_ns_per_query": "305"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "parsing_ns_per_query": "3670", "dictionary_search_ns_per_query": "12", "conjunctive_search_ns_per_query": "6644", "reporting_ns_per_query": "714"}

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "5667"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "19144"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "18886"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "18109"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "16030"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "14423"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "13418"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "12779"}