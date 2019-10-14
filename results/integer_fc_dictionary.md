#### Results on the AOL querylog.

	pibiri@rubino:~/autocomplete/build$ ./benchmark_integer_fc_dictionary ../test_data/aol/aol.completions 1000000
	2019-10-14 15:28:12: building integer_fc_dictionary with bucket size 4...
	2019-10-14 15:28:14: DONE
	using 129855836 bytes
	extract: 102.787 [ns/string]
	2019-10-14 15:28:15: building integer_fc_dictionary with bucket size 8...
	2019-10-14 15:28:18: DONE
	using 112779868 bytes
	extract: 98.9981 [ns/string]
	2019-10-14 15:28:19: building integer_fc_dictionary with bucket size 16...
	2019-10-14 15:28:21: DONE
	using 102740006 bytes
	extract: 103.745 [ns/string]
	2019-10-14 15:28:22: building integer_fc_dictionary with bucket size 32...
	2019-10-14 15:28:24: DONE
	using 97266766 bytes
	extract: 136.042 [ns/string]
	2019-10-14 15:28:26: building integer_fc_dictionary with bucket size 64...
	2019-10-14 15:28:28: DONE
	using 94397632 bytes
	extract: 207.699 [ns/string]
	2019-10-14 15:28:30: building integer_fc_dictionary with bucket size 128...
	2019-10-14 15:28:32: DONE
	using 92933198 bytes
	extract: 354.622 [ns/string]
	2019-10-14 15:28:36: building integer_fc_dictionary with bucket size 256...
	2019-10-14 15:28:38: DONE
	using 92192244 bytes
	extract: 651.357 [ns/string]