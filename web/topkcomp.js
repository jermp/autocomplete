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
        formatResult: function(suggestion, currentValue) {
            // Do not replace anything if the current value is empty
            if (!currentValue) {
                return suggestion.value;
            }

            // match any tokens, not just prefix
            var keywords = currentValue.trim().split(/\s+/).join('|');

            return suggestion.value
                .replace(new RegExp("(" + keywords + ")", "gi"), '<strong>$1<\/strong>')
                .replace(/&/g, '&amp;')
                .replace(/</g, '&lt;')
                .replace(/>/g, '&gt;')
                .replace(/"/g, '&quot;')
                .replace(/&lt;(\/?strong)&gt;/g, '<$1>');
        },
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
