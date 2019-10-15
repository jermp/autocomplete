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
	
	+ Elias-Fano and forward_index2
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
	
#### Solution 2

	using 0.377843 [GiB]
	  completions: 0.0956838 [GiB] (25.3237%)
	  unsorted docs list: 0.0409812 [GiB] (10.8461%)
	  unsorted minimal docs list: 0.0154568 [GiB] (4.09079%)
	  dictionary: 0.0330574 [GiB] (8.74898%)
	  inverted index: 0.154881 [GiB] (40.9907%)
	  map from docid to lexid: 0.0377834 [GiB] (9.99975%)
	  
	  
	+ Elias-Fano
	using 0.282347 [GiB]
	  completions: 0.0956841 [GiB] (33.8888%)
	  unsorted docs list: 0.0409812 [GiB] (14.5145%)
	  unsorted minimal docs list: 0.0154568 [GiB] (5.47438%)
	  dictionary: 0.0328479 [GiB] (11.6339%)
	  inverted index: 0.0595939 [GiB] (21.1066%)
		data: 15.7999 [bpi]
		pointers: 1.20819 [bpi]
	  map from docid to lexid: 0.0377834 [GiB] (13.3819%)
  	  

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
      
 
#### Solution 2

	using 0.263256 [GiB]
	  completions: 0.0681158 [GiB] (25.8744%)
	  unsorted docs list: 0.0286179 [GiB] (10.8708%)
	  unsorted minimal docs list: 0.0104689 [GiB] (3.97669%)
	  dictionary: 0.0220881 [GiB] (8.39036%)
	  inverted index: 0.107578 [GiB] (40.8643%)
	  map from docid to lexid: 0.0263876 [GiB] (10.0236%)