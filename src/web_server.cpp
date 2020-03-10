#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include "constants.hpp"
#include "types.hpp"
#include "probe.hpp"

#include "../external/mongoose/mongoose.h"

// -- Helper function to escape output
// http://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c/33799784#33799784
std::string escape_json(std::string const& s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        if (*c == '"' || *c == '\\' || ('\x00' <= *c && *c <= '\x1f')) {
            o << "\\u" << std::hex << std::setw(4) << std::setfill('0')
              << (int)*c;
        } else {
            o << *c;
        }
    }
    return o.str();
}
// --

using namespace autocomplete;

typedef ef_autocomplete_type1 topk_index_type;

static std::string s_http_port("8000");
static struct mg_serve_http_opts s_http_server_opts;
static topk_index_type topk_index;

static void ev_handler(struct mg_connection* nc, int ev, void* p) {
    if (ev == MG_EV_HTTP_REQUEST) {
        struct http_message* hm = (struct http_message*)p;
        std::string uri = std::string(hm->uri.p, (hm->uri.p) + (hm->uri.len));

        if (uri == "/topcomp") {
            std::string query = "";
            size_t k = 10;
            char query_buf[constants::MAX_NUM_CHARS_PER_QUERY];
            int query_len = mg_get_http_var(&(hm->query_string), "q", query_buf,
                                            constants::MAX_NUM_CHARS_PER_QUERY);
            if (query_len > 0) {
                query = std::string(query_buf, query_buf + query_len);
            }
            char k_buf[16];
            int k_len = mg_get_http_var(&(hm->query_string), "k", k_buf, 16);
            if (k_len > 0) {
                k = std::stoull(std::string(k_buf, k_buf + k_len));
            }

            std::string data;
            nop_probe probe;
            // auto it = topk_index.topk(query, k probe);
            // auto it = topk_index.prefix_topk(query, k, probe);
            auto it = topk_index.conjunctive_topk(query, k, probe);
            if (it.empty()) {
                data = "{\"suggestions\":[\"value\":\"\",\"data\":\"\"]}\n";
            } else {
                data = "{\"suggestions\":[";
                for (size_t i = 0; i != it.size(); ++i, ++it) {
                    auto completion = *it;
                    if (i > 0) data += ",";
                    data += "{\"value\":\"" +
                            escape_json(std::string(completion.string.begin,
                                                    completion.string.end)) +
                            "\",";
                    data += "\"data\":\"" + std::to_string(i) + "\"}";
                }
                data += "]}\n";
            }
            /* Send headers */
            mg_printf(nc, "%s",
                      "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_printf_http_chunk(nc, data.c_str(), data.size());
            mg_send_http_chunk(nc, "",
                               0);  // send empty chunk, the end of response
        } else {
            mg_serve_http(nc, (struct http_message*)p, s_http_server_opts);
        }
    }
}

int main(int argc, char** argv) {
    int mandatory = 2;
    if (argc < mandatory + 1) {
        std::cout << argv[0] << " <port> <index_filename>" << std::endl;
        return 1;
    }

    s_http_port = argv[1];
    char const* index_filename = argv[2];
    essentials::load(topk_index, index_filename);

    struct mg_mgr mgr;
    struct mg_connection* nc;

    mg_mgr_init(&mgr, NULL);
    nc = mg_bind(&mgr, s_http_port.c_str(), ev_handler);

    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = "../web";
    s_http_server_opts.enable_directory_listing = "no";

    printf("Starting web server on port %s\n", s_http_port.c_str());

    while (true) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);

    return 0;
}
