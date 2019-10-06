/*jslint  browser: true, white: true, plusplus: true */

$(function () {
    'use strict';

    // Initialize ajax autocomplete:
    $('#autocomplete').autocomplete({
        noCache: true,
        serviceUrl: '/topcomp',
        dataType: 'json',
        minChars: '0',
        params: {k: 10},
        paramName: 'q',
        lookupFilter: function(suggestion, originalQuery, queryLowerCase) {
            var re = new RegExp('\\b' + $.Autocomplete.utils.escapeRegExChars(queryLowerCase), 'gi');
            return re.test(suggestion.value);
        },
        onSelect: function(suggestion) {
            //window.location = 'http://google.com/maps?q=' + suggestion.value;
        }
    });
});
