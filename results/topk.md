Top-k
-----------------

Executing queries shuffled at random, for k = 7.

Average among 10 runs.

### AOL

#### Solution 1

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "5062"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "6725"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "24960"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "32761"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "31450"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "28812"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "25978"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "22785"}

#### Solution 2

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "5812"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "12703"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "27307"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "33476"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "31403"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "28718"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "25728"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "22419"}

#### Solution 3

	{"num_terms_per_query": "1", "num_queries": "1000", "ns_per_query": "5899"}
	{"num_terms_per_query": "2", "num_queries": "1000", "ns_per_query": "12282007"}
	{"num_terms_per_query": "3", "num_queries": "1000", "ns_per_query": "18393403"}
	{"num_terms_per_query": "4", "num_queries": "1000", "ns_per_query": "15212918"}
	{"num_terms_per_query": "5", "num_queries": "1000", "ns_per_query": "11852012"}
	{"num_terms_per_query": "6", "num_queries": "1000", "ns_per_query": "7781194"}
	{"num_terms_per_query": "7", "num_queries": "1000", "ns_per_query": "7939661"}
	{"num_terms_per_query": "8+", "num_queries": "1000", "ns_per_query": "6980226"}

#### Solution 4 with c = 0.1

	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6491"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "1561428"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "4761415"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "5352705"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "6598943"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "6112026"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "6369838"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "6283191"}



### MSN

#### Solution 1

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "5823"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "6251"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "16502"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "18380"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "17044"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "15622"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "14709"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "14323"}

#### Solution 2

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "6837"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "14469"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "18670"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "19144"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "17109"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "15738"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "14810"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "14260"}
	
	
#### Solution 3

	
	{"num_terms_per_query": "1", "num_queries": "1000", "ns_per_query": "6666"}
	{"num_terms_per_query": "2", "num_queries": "1000", "ns_per_query": "6635754"}
	{"num_terms_per_query": "3", "num_queries": "1000", "ns_per_query": "8612266"}
	{"num_terms_per_query": "4", "num_queries": "1000", "ns_per_query": "5290905"}
	{"num_terms_per_query": "5", "num_queries": "1000", "ns_per_query": "3939319"}
	{"num_terms_per_query": "6", "num_queries": "1000", "ns_per_query": "3035556"}
	{"num_terms_per_query": "7", "num_queries": "1000", "ns_per_query": "3106875"}
	{"num_terms_per_query": "8+", "num_queries": "1000", "ns_per_query": "3089917"}
	
#### Solution 4 with c = 0.1
	
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "7496"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "1280652"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "3181191"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "3722226"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "4056810"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "4130288"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "4282750"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "4205507"}