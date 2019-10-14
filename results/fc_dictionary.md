#### Results on the AOL querylog.

	pibiri@rubino:~/autocomplete/build$ ./benchmark_fc_dictionary ../test_data/aol/aol.completions 1000000 < ../test_data/aol/aol.completions.dict
	2019-10-14 14:10:37: loading queries...
	2019-10-14 14:10:37: loaded 1000000 queries
	2019-10-14 14:10:37: building fc_dictionary with bucket size 4...
	2019-10-14 14:10:37: DONE
	using 42938890 bytes
	locate: 125.317 [ns/string]
	extract: 9.6834 [ns/string]
	2019-10-14 14:10:39: building fc_dictionary with bucket size 8...
	2019-10-14 14:10:39: DONE
	using 38111527 bytes
	locate: 135.135 [ns/string]
	extract: 15.647 [ns/string]
	2019-10-14 14:10:41: building fc_dictionary with bucket size 16...
	2019-10-14 14:10:41: DONE
	using 35270205 bytes
	locate: 160.462 [ns/string]
	extract: 28.1554 [ns/string]
	2019-10-14 14:10:43: building fc_dictionary with bucket size 32...
	2019-10-14 14:10:43: DONE
	using 33722303 bytes
	locate: 217.425 [ns/string]
	extract: 53.1404 [ns/string]
	2019-10-14 14:10:46: building fc_dictionary with bucket size 64...
	2019-10-14 14:10:47: DONE
	using 32910194 bytes
	locate: 338.898 [ns/string]
	extract: 105.197 [ns/string]
	2019-10-14 14:10:51: building fc_dictionary with bucket size 128...
	2019-10-14 14:10:52: DONE
	using 32496375 bytes
	locate: 589.852 [ns/string]
	extract: 205.954 [ns/string]
	2019-10-14 14:11:00: building fc_dictionary with bucket size 256...
	2019-10-14 14:11:00: DONE
	using 32286042 bytes
	locate: 1089.73 [ns/string]
	extract: 394.438 [ns/string]