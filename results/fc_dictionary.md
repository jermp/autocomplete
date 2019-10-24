#### Results on the AOL querylog.

	pibiri@rubino:~/autocomplete/build$ ./benchmark_fc_dictionary ../test_data/aol/aol.completions 1000000 < ../test_data/aol/aol.completions.dict_queries.1M.shuffled 
	2019-10-24 11:11:49: loading queries...
	2019-10-24 11:11:49: loaded 1000000 queries
	2019-10-24 11:11:49: building fc_dictionary with bucket size 4...
	2019-10-24 11:11:50: DONE
	using 42938890 bytes
	locate: 557.091 [ns/string]
	extract: 168.772 [ns/string]
	locate_prefix-0%: 213.453 [ns/string]
	locate_prefix-25%: 794.612 [ns/string]
	locate_prefix-50%: 1064.44 [ns/string]
	locate_prefix-75%: 912.04 [ns/string]
	locate_prefix-100%: 702.745 [ns/string]
	2019-10-24 11:12:12: building fc_dictionary with bucket size 8...
	2019-10-24 11:12:12: DONE
	using 38111527 bytes
	locate: 511.503 [ns/string]
	extract: 152.331 [ns/string]
	locate_prefix-0%: 223.374 [ns/string]
	locate_prefix-25%: 686.093 [ns/string]
	locate_prefix-50%: 873.161 [ns/string]
	locate_prefix-75%: 758.029 [ns/string]
	locate_prefix-100%: 638.576 [ns/string]
	2019-10-24 11:12:32: building fc_dictionary with bucket size 16...
	2019-10-24 11:12:32: DONE
	using 35270205 bytes
	locate: 478.592 [ns/string]
	extract: 139.109 [ns/string]
	locate_prefix-0%: 228.416 [ns/string]
	locate_prefix-25%: 662.483 [ns/string]
	locate_prefix-50%: 769.227 [ns/string]
	locate_prefix-75%: 685.358 [ns/string]
	locate_prefix-100%: 615.757 [ns/string]
	2019-10-24 11:12:51: building fc_dictionary with bucket size 32...
	2019-10-24 11:12:51: DONE
	using 33722303 bytes
	locate: 484.72 [ns/string]
	extract: 150.21 [ns/string]
	locate_prefix-0%: 273.595 [ns/string]
	locate_prefix-25%: 717.559 [ns/string]
	locate_prefix-50%: 790.342 [ns/string]
	locate_prefix-75%: 728.409 [ns/string]
	locate_prefix-100%: 681.921 [ns/string]
	2019-10-24 11:13:11: building fc_dictionary with bucket size 64...
	2019-10-24 11:13:11: DONE
	using 32910194 bytes
	locate: 585.835 [ns/string]
	extract: 194.183 [ns/string]
	locate_prefix-0%: 667.159 [ns/string]
	locate_prefix-25%: 962.096 [ns/string]
	locate_prefix-50%: 1056.04 [ns/string]
	locate_prefix-75%: 1014.63 [ns/string]
	locate_prefix-100%: 978.718 [ns/string]
	2019-10-24 11:13:39: building fc_dictionary with bucket size 128...
	2019-10-24 11:13:39: DONE
	using 32496375 bytes
	locate: 810.282 [ns/string]
	extract: 286.967 [ns/string]
	locate_prefix-0%: 574.352 [ns/string]
	locate_prefix-25%: 1248.92 [ns/string]
	locate_prefix-50%: 1435.28 [ns/string]
	locate_prefix-75%: 1419.18 [ns/string]
	locate_prefix-100%: 1398.48 [ns/string]
	2019-10-24 11:14:16: building fc_dictionary with bucket size 256...
	2019-10-24 11:14:16: DONE
	using 32286042 bytes
	locate: 1281.09 [ns/string]
	extract: 470.922 [ns/string]
	locate_prefix-0%: 1065.07 [ns/string]
	locate_prefix-25%: 2099.35 [ns/string]
	locate_prefix-50%: 2387.39 [ns/string]
	locate_prefix-75%: 2407.04 [ns/string]
	locate_prefix-100%: 2403.04 [ns/string]