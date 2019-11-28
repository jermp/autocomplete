#include "test_common.hpp"

using namespace autocomplete;

TEST_CASE("test compact_forward_index::iterator") {
    char const* output_filename = testing::tmp_filename.c_str();
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    {
        compact_forward_index::builder builder(params);
        compact_forward_index index;
        builder.build(index);
        REQUIRE(index.num_docs() == params.universe);
        REQUIRE(index.num_terms() == params.num_terms);
        essentials::save<compact_forward_index>(index, output_filename);
    }

    {
        compact_forward_index index;
        essentials::load(index, output_filename);
        REQUIRE(index.num_docs() == params.universe);
        REQUIRE(index.num_terms() == params.num_terms);

        std::ifstream input((params.collection_basename + ".forward").c_str(),
                            std::ios_base::in);
        for (uint64_t i = 0; i != index.num_terms(); ++i) {
            auto it = index.iterator(i);
            uint32_t n = 0;
            input >> n;
            REQUIRE_MESSAGE(n == it.size(), "list has size " << it.size()
                                                             << " instead of "
                                                             << n);
            for (uint64_t k = 0; k != n; ++k, ++it) {
                id_type expected;
                input >> expected;
                auto got = *it;
                REQUIRE_MESSAGE(got == expected,
                                "got " << got << " but expected " << expected);
            }
        }
        input.close();

        std::remove(output_filename);
    }
};
