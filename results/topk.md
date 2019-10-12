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

#### Solution 4

	c = 0.005
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6556"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "5189102"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "24462147"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "15558504"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "25025950"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "12735179"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "19698416"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "19113746"}

	c = 0.025
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6578"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "1507044"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "6736209"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "4803337"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "7270651"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "4219520"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "5967962"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "5831412"}

	c = 0.05
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6513"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "1242104"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "4493104"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "4021800"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "5274711"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "4015044"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "4707150"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "4714434"}

	c = 0.1
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6491"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "1561428"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "4761415"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "5352705"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "6598943"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "6112026"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "6369838"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "6283191"}

	c = 0.2
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6555"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "2722523"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "7328397"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "9298665"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "9935672"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "10679862"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "10888510"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "11257755"}

	c = 0.3
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6521"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "4177212"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "10385221"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "13190926"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "14618246"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "14687532"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "15307410"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "14996264"}

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