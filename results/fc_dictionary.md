#### Results on the AOL querylog.

	pibiri@rubino:~/autocomplete/build$ ./benchmark_fc_dictionary ../test_data/aol/aol.completions 1000000 < ../test_data/aol/aol.completions.dict
	2019-10-14 13:37:11: loading queries...
	2019-10-14 13:37:11: loaded 1000000 queries
	2019-10-14 13:37:11: building fc_dictionary with bucket size 4...
	2019-10-14 13:37:12: DONE
	using 42938762 bytes
	locate: 137.05 [ns/string]
	extract: 20.0646 [ns/string]
	2019-10-14 13:37:13: building fc_dictionary with bucket size 8...
	2019-10-14 13:37:14: DONE
	using 38111399 bytes
	locate: 151.646 [ns/string]
	extract: 40.733 [ns/string]
	2019-10-14 13:37:16: building fc_dictionary with bucket size 16...
	2019-10-14 13:37:16: DONE
	using 35270077 bytes
	locate: 182.845 [ns/string]
	extract: 66.8254 [ns/string]
	2019-10-14 13:37:19: building fc_dictionary with bucket size 32...
	2019-10-14 13:37:19: DONE
	using 33722175 bytes
	locate: 259.57 [ns/string]
	extract: 111.641 [ns/string]
	2019-10-14 13:37:23: building fc_dictionary with bucket size 64...
	2019-10-14 13:37:23: DONE
	using 32910066 bytes
	locate: 415.206 [ns/string]
	extract: 195.543 [ns/string]
	2019-10-14 13:37:30: building fc_dictionary with bucket size 128...
	2019-10-14 13:37:30: DONE
	using 32496247 bytes
	locate: 722.426 [ns/string]
	extract: 370.092 [ns/string]
	2019-10-14 13:37:42: building fc_dictionary with bucket size 256...
	2019-10-14 13:37:42: DONE
	using 32285914 bytes
	locate: 1342.53 [ns/string]
	extract: 715.244 [ns/string]