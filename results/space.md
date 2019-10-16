AOL 2006 query log
------------------

10,142,395 distinct queries, whose ids have been assigned
in decreasing frequency order (ties broken lexicographically).

#### Solution 1

	using 1.05555 [GiB]
	  completions: 0.520278 [GiB] (49.2899%)
	  unsorted docs list: 0.0409812 [GiB] (3.88246%)
	  unsorted minimal docs list: 0.0154568 [GiB] (1.46434%)
	  dictionary: 0.0328479 [GiB] (3.11194%)
	  inverted index: 0.144273 [GiB] (13.6681%)
		data: 33.0401 [bpi]
		pointers: 8.13526 [bpi]
	  forward index: 0.30171 [GiB] (28.5833%)
		data: 42.6801 [bpi]
		pointers: 42.8379 [bpi]
	
	
	+ Elias-Fano
	using 0.370675 [GiB]
	  completions: 0.0867222 [GiB] (23.3958%)
	  unsorted docs list: 0.0409812 [GiB] (11.0558%)
	  unsorted minimal docs list: 0.0154568 [GiB] (4.1699%)
	  dictionary: 0.0328479 [GiB] (8.86166%)
	  inverted index: 0.0595939 [GiB] (16.0771%)
		data: 15.7999 [bpi]
		pointers: 1.20819 [bpi]
	  forward index: 0.135073 [GiB] (36.4397%)
		data: 32.866 [bpi]
		pointers: 5.41964 [bpi]
	
	+ Elias-Fano and compact_forward_index
	using 0.318008 [GiB]
	  completions: 0.0867222 [GiB] (27.2704%)
	  unsorted docs list: 0.0409812 [GiB] (12.8868%)
	  unsorted minimal docs list: 0.0154568 [GiB] (4.86049%)
	  dictionary: 0.0328479 [GiB] (10.3293%)
	  inverted index: 0.0595939 [GiB] (18.7397%)
		data: 15.7999 [bpi]
		pointers: 1.20819 [bpi]
	  forward index: 0.0824065 [GiB] (25.9133%)
		data: 22 [bpi]
		pointers: 1.35762 [bpi]
	
	+ Elias-Fano and delta_forward_index
	using 0.350595 [GiB]
	  completions: 0.086722 [GiB] (24.7356%)
	  unsorted docs list: 0.0409812 [GiB] (11.689%)
	  unsorted minimal docs list: 0.0154568 [GiB] (4.40872%)
	  dictionary: 0.0328479 [GiB] (9.36919%)
		data: 69.9866 [bps]
		pointers: 3.76476 [bps]
	  inverted index: 0.0595939 [GiB] (16.9979%)
		data: 15.7999 [bpi]
		pointers: 1.20819 [bpi]
	  forward index: 0.114994 [GiB] (32.7995%)
		data: 29.6008 [bpi]
		pointers: 2.99348 [bpi]
	
	+ Elias-Fano + compact_forward_index + compact_unsorted_lists
	using 0.304999 [GiB]
	  completions: 0.086722 [GiB] (28.4335%)
	  unsorted docs list: 0.0315353 [GiB] (10.3395%)
	  unsorted minimal docs list: 0.0118937 [GiB] (3.89958%)
	  dictionary: 0.0328479 [GiB] (10.7698%)
		data: 69.9866 [bps]
		pointers: 3.76476 [bps]
	  inverted index: 0.0595939 [GiB] (19.539%)
		data: 15.7999 [bpi]
		pointers: 1.20819 [bpi]
	  forward index: 0.0824065 [GiB] (27.0186%)
		data: 22 [bpi]
		pointers: 1.35762 [bpi]
	
#### Solution 2

	using 0.377843 [GiB]
	  completions: 0.0956838 [GiB] (25.3237%)
	  unsorted docs list: 0.0409812 [GiB] (10.8461%)
	  unsorted minimal docs list: 0.0154568 [GiB] (4.09079%)
	  dictionary: 0.0330574 [GiB] (8.74898%)
	  inverted index: 0.154881 [GiB] (40.9907%)
	  map from docid to lexid: 0.0377834 [GiB] (9.99975%)
	  
	  
	+ Elias-Fano
	using 0.259893 [GiB]
	  completions: 0.0956841 [GiB] (36.8168%)
		data: 73.5086 [bps]
		pointers: 7.52944 [bps]
	  unsorted docs list: 0.0315353 [GiB] (12.134%)
	  unsorted minimal docs list: 0.0118937 [GiB] (4.57639%)
	  dictionary: 0.0328479 [GiB] (12.639%)
		data: 69.9866 [bps]
		pointers: 3.76476 [bps]
	  inverted index: 0.0595939 [GiB] (22.9302%)
		data: 15.7999 [bpi]
		pointers: 1.20819 [bpi]
	  map from docid to lexid: 0.0283376 [GiB] (10.9036%)
  	  

MSN 2006 query log
------------------

7,083,363 distinct queries, whose ids have been assigned
in decreasing frequency order (ties broken lexicographically).

#### Solution 1

    using 0.769592 [GiB]
      completion trie: 0.370163 [GiB] (48.0986%)
      unsorted docs list: 0.0286179 [GiB] (3.71858%)
      unsorted minimal docs list: 0.0104689 [GiB] (1.36031%)
      dictionary: 0.0220881 [GiB] (2.87011%)
      inverted index: 0.107578 [GiB] (13.9785%)
      forward index: 0.230677 [GiB] (29.9739%)
      
     + compression 
	 using 0.213269 [GiB]
	  completions: 0.0617906 [GiB] (28.973%)
	  unsorted docs list: 0.0211964 [GiB] (9.9388%)
	  unsorted minimal docs list: 0.00775427 [GiB] (3.6359%)
	  dictionary: 0.0219463 [GiB] (10.2904%)
		data: 68.9954 [bps]
		pointers: 3.7648 [bps]
	  inverted index: 0.0429281 [GiB] (20.1286%)
		data: 16.2938 [bpi]
		pointers: 1.1785 [bpi]
	  forward index: 0.0576538 [GiB] (27.0333%)
		data: 22 [bpi]
		pointers: 1.35605 [bpi]
 
#### Solution 2

	using 0.263256 [GiB]
	  completions: 0.0681158 [GiB] (25.8744%)
	  unsorted docs list: 0.0286179 [GiB] (10.8708%)
	  unsorted minimal docs list: 0.0104689 [GiB] (3.97669%)
	  dictionary: 0.0220881 [GiB] (8.39036%)
	  inverted index: 0.107578 [GiB] (40.8643%)
	  map from docid to lexid: 0.0263876 [GiB] (10.0236%)
	  
	+ compression
	using 0.180907 [GiB]
	  completions: 0.0681161 [GiB] (37.6525%)
		data: 75.0743 [bps]
		pointers: 7.52946 [bps]
	  unsorted docs list: 0.0211964 [GiB] (11.7167%)
	  unsorted minimal docs list: 0.00775427 [GiB] (4.28633%)
	  dictionary: 0.0219463 [GiB] (12.1312%)
		data: 68.9954 [bps]
		pointers: 3.7648 [bps]
	  inverted index: 0.0429281 [GiB] (23.7293%)
		data: 16.2938 [bpi]
		pointers: 1.1785 [bpi]
	  map from docid to lexid: 0.0189661 [GiB] (10.4839%)