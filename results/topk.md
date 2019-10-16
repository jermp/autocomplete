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

	+ Elias-Fano
	{"num_terms_per_query": "1", "num_queries": "10000", "ns_per_query": "5614"}
	{"num_terms_per_query": "2", "num_queries": "10000", "ns_per_query": "9767"}
	{"num_terms_per_query": "3", "num_queries": "10000", "ns_per_query": "26999"}
	{"num_terms_per_query": "4", "num_queries": "10000", "ns_per_query": "35428"}
	{"num_terms_per_query": "5", "num_queries": "10000", "ns_per_query": "36073"}
	{"num_terms_per_query": "6", "num_queries": "10000", "ns_per_query": "31718"}
	{"num_terms_per_query": "7", "num_queries": "10000", "ns_per_query": "29992"}
	{"num_terms_per_query": "8+", "num_queries": "10000", "ns_per_query": "27313"}
	
	+ Elias-Fano and forward_index2
	{"num_terms_per_query": "1", "num_queries": "10000", "ns_per_query": "5336"}
	{"num_terms_per_query": "2", "num_queries": "10000", "ns_per_query": "7573"}
	{"num_terms_per_query": "3", "num_queries": "10000", "ns_per_query": "26278"}
	{"num_terms_per_query": "4", "num_queries": "10000", "ns_per_query": "35664"}
	{"num_terms_per_query": "5", "num_queries": "10000", "ns_per_query": "35189"}
	{"num_terms_per_query": "6", "num_queries": "10000", "ns_per_query": "32033"}
	{"num_terms_per_query": "7", "num_queries": "10000", "ns_per_query": "29950"}
	{"num_terms_per_query": "8+", "num_queries": "10000", "ns_per_query": "27332"}
	
#### Solution 2

	{"num_terms_per_query": "1", "num_queries": "50000", "ns_per_query": "5812"}
	{"num_terms_per_query": "2", "num_queries": "50000", "ns_per_query": "12703"}
	{"num_terms_per_query": "3", "num_queries": "50000", "ns_per_query": "27307"}
	{"num_terms_per_query": "4", "num_queries": "50000", "ns_per_query": "33476"}
	{"num_terms_per_query": "5", "num_queries": "50000", "ns_per_query": "31403"}
	{"num_terms_per_query": "6", "num_queries": "50000", "ns_per_query": "28718"}
	{"num_terms_per_query": "7", "num_queries": "50000", "ns_per_query": "25728"}
	{"num_terms_per_query": "8+", "num_queries": "50000", "ns_per_query": "22419"}

	+ Elias-Fano
	{"num_terms_per_query": "1", "num_queries": "10000", "ns_per_query": "5609"}
	{"num_terms_per_query": "2", "num_queries": "10000", "ns_per_query": "10894"}
	{"num_terms_per_query": "3", "num_queries": "10000", "ns_per_query": "27311"}
	{"num_terms_per_query": "4", "num_queries": "10000", "ns_per_query": "34780"}
	{"num_terms_per_query": "5", "num_queries": "10000", "ns_per_query": "33849"}
	{"num_terms_per_query": "6", "num_queries": "10000", "ns_per_query": "30319"}
	{"num_terms_per_query": "7", "num_queries": "10000", "ns_per_query": "28181"}
	{"num_terms_per_query": "8+", "num_queries": "10000", "ns_per_query": "24757"}
	
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
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6593"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "756944"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "2188766"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "1920720"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "2398355"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "1711205"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "2195672"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "2115028"}

	c = 0.01
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6610"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "739838"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "2147339"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "1988980"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "2440435"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "1858965"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "2304761"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "2254481"}

	c = 0.025
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6528"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "828082"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "2422803"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "2482018"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "2970064"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "2542134"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "2972710"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "2924603"}

	c = 0.05
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6508"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "1059938"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "3046716"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "3528723"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "4037290"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "3850329"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "4371489"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "4648349"}

	c = 0.1
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6584"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "1600869"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "4501125"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "5562030"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "6634491"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "6768321"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "7124462"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "7733525"}

	c = 0.2
	{"num_terms_per_query": "1", "num_queries": "100", "ns_per_query": "6589"}
	{"num_terms_per_query": "2", "num_queries": "100", "ns_per_query": "2831409"}
	{"num_terms_per_query": "3", "num_queries": "100", "ns_per_query": "7641806"}
	{"num_terms_per_query": "4", "num_queries": "100", "ns_per_query": "9881857"}
	{"num_terms_per_query": "5", "num_queries": "100", "ns_per_query": "11138148"}
	{"num_terms_per_query": "6", "num_queries": "100", "ns_per_query": "11643908"}
	{"num_terms_per_query": "7", "num_queries": "100", "ns_per_query": "11966417"}
	{"num_terms_per_query": "8+", "num_queries": "100", "ns_per_query": "12460833"}

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