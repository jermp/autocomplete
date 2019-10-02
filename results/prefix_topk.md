Prefix top-k
------------

Executing queries shuffled at random, for k = 7 (as Google).
Average among 10 runs.

### AOL

    {"num_terms_per_query": "1", "num_queries": "1000000", "parsing_ns_per_query": "45", "completion_trie_search_ns_per_query": "3", "topk_rmq_ns_per_query": "23", "reporting_ns_per_query": "145"}
    {"num_terms_per_query": "2", "num_queries": "1000000", "parsing_ns_per_query": "789", "completion_trie_search_ns_per_query": "40", "topk_rmq_ns_per_query": "23", "reporting_ns_per_query": "293"}
    {"num_terms_per_query": "3", "num_queries": "1000000", "parsing_ns_per_query": "1101", "completion_trie_search_ns_per_query": "258", "topk_rmq_ns_per_query": "6", "reporting_ns_per_query": "156"}
    {"num_terms_per_query": "4", "num_queries": "1000000", "parsing_ns_per_query": "1527", "completion_trie_search_ns_per_query": "576", "topk_rmq_ns_per_query": "3", "reporting_ns_per_query": "112"}
    {"num_terms_per_query": "5", "num_queries": "858693", "parsing_ns_per_query": "2048", "completion_trie_search_ns_per_query": "811", "topk_rmq_ns_per_query": "1", "reporting_ns_per_query": "138"}
    {"num_terms_per_query": "6", "num_queries": "448590", "parsing_ns_per_query": "2363", "completion_trie_search_ns_per_query": "943", "topk_rmq_ns_per_query": "1", "reporting_ns_per_query": "229"}
    {"num_terms_per_query": "7", "num_queries": "226502", "parsing_ns_per_query": "2836", "completion_trie_search_ns_per_query": "1021", "topk_rmq_ns_per_query": "1", "reporting_ns_per_query": "400"}
    {"num_terms_per_query": "8+", "num_queries": "284439", "parsing_ns_per_query": "3859", "completion_trie_search_ns_per_query": "1245", "topk_rmq_ns_per_query": "1", "reporting_ns_per_query": "795"}

### MSN

    {"num_terms_per_query": "1", "num_queries": "1000000", "parsing_ns_per_query": "49", "completion_trie_search_ns_per_query": "3", "topk_rmq_ns_per_query": "32", "reporting_ns_per_query": "190"}
    {"num_terms_per_query": "2", "num_queries": "1000000", "parsing_ns_per_query": "683", "completion_trie_search_ns_per_query": "14", "topk_rmq_ns_per_query": "16", "reporting_ns_per_query": "206"}
    {"num_terms_per_query": "3", "num_queries": "1000000", "parsing_ns_per_query": "1030", "completion_trie_search_ns_per_query": "114", "topk_rmq_ns_per_query": "4", "reporting_ns_per_query": "105"}
    {"num_terms_per_query": "4", "num_queries": "1000000", "parsing_ns_per_query": "1453", "completion_trie_search_ns_per_query": "390", "topk_rmq_ns_per_query": "2", "reporting_ns_per_query": "77"}
    {"num_terms_per_query": "5", "num_queries": "598663", "parsing_ns_per_query": "1989", "completion_trie_search_ns_per_query": "629", "topk_rmq_ns_per_query": "0", "reporting_ns_per_query": "93"}
    {"num_terms_per_query": "6", "num_queries": "284414", "parsing_ns_per_query": "2334", "completion_trie_search_ns_per_query": "835", "topk_rmq_ns_per_query": "1", "reporting_ns_per_query": "157"}
    {"num_terms_per_query": "7", "num_queries": "129684", "parsing_ns_per_query": "2748", "completion_trie_search_ns_per_query": "947", "topk_rmq_ns_per_query": "0", "reporting_ns_per_query": "297"}
    {"num_terms_per_query": "8+", "num_queries": "133760", "parsing_ns_per_query": "3696", "completion_trie_search_ns_per_query": "1084", "topk_rmq_ns_per_query": "0", "reporting_ns_per_query": "722"}