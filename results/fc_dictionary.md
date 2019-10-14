#### Results on the AOL querylog.

	pibiri@rubino:~/autocomplete/build$ ./benchmark_fc_dictionary ../test_data/aol/aol.completions 1000000 < ../test_data/aol/aol.completions.dict_queries.1M.shuffled 
	2019-10-14 14:54:24: loading queries...
	2019-10-14 14:54:24: loaded 1000000 queries
	2019-10-14 14:54:24: building fc_dictionary with bucket size 4...
	2019-10-14 14:54:25: DONE
	using 42938890 bytes
	locate: 559.666 [ns/string]
	extract: 165.846 [ns/string]
	2019-10-14 14:54:32: building fc_dictionary with bucket size 8...
	2019-10-14 14:54:33: DONE
	using 38111527 bytes
	locate: 515.359 [ns/string]
	extract: 151.121 [ns/string]
	2019-10-14 14:54:40: building fc_dictionary with bucket size 16...
	2019-10-14 14:54:40: DONE
	using 35270205 bytes
	locate: 474.319 [ns/string]
	extract: 138.07 [ns/string]
	2019-10-14 14:54:47: building fc_dictionary with bucket size 32...
	2019-10-14 14:54:47: DONE
	using 33722303 bytes
	locate: 490 [ns/string]
	extract: 150.671 [ns/string]
	2019-10-14 14:54:54: building fc_dictionary with bucket size 64...
	2019-10-14 14:54:54: DONE
	using 32910194 bytes
	locate: 585.408 [ns/string]
	extract: 197.131 [ns/string]
	2019-10-14 14:55:03: building fc_dictionary with bucket size 128...
	2019-10-14 14:55:03: DONE
	using 32496375 bytes
	locate: 812.441 [ns/string]
	extract: 293.022 [ns/string]
	2019-10-14 14:55:15: building fc_dictionary with bucket size 256...
	2019-10-14 14:55:15: DONE
	using 32286042 bytes
	locate: 1283.83 [ns/string]
	extract: 485.985 [ns/string]