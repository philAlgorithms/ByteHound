#include "test_harness.hpp"

#include "tinycfg/tinycfg.hpp"

TEST(serializer_canonical_output) {
    tinycfg::Document doc;
    const auto parse_result = tinycfg::parse(R"(server {
        host = "localhost";
        port = 8080;
    })",
                                             doc);
    EXPECT_TRUE(parse_result.ok());

    std::string output;
    const auto serialize_result = tinycfg::serialize(doc, output);
    EXPECT_TRUE(serialize_result.ok());
    EXPECT_NE(output.find("server {"), std::string::npos);
    EXPECT_NE(output.find("host = \"localhost\";"), std::string::npos);
    EXPECT_NE(output.find("port = 8080;"), std::string::npos);
}

TEST(serializer_roundtrip_equality) {
    const char* input = R"(app {
        name = "ByteHound";
        tags = ["fuzz", "parser"];
        meta = { enabled = true; retries = 3; };
    })";
    tinycfg::Document first;
    EXPECT_TRUE(tinycfg::parse(input, first).ok());

    std::string serialized;
    EXPECT_TRUE(tinycfg::serialize(first, serialized).ok());

    tinycfg::Document second;
    EXPECT_TRUE(tinycfg::parse(serialized, second).ok());
    EXPECT_TRUE(tinycfg::structural_equal(first, second));

    std::string reserialized;
    EXPECT_TRUE(tinycfg::serialize(second, reserialized).ok());
    EXPECT_EQ(serialized, reserialized);
}

TEST(serializer_escapes_special_characters) {
    tinycfg::Document doc;
    tinycfg::Object section;
    section["text"] = tinycfg::Value{tinycfg::StringValue{"line\n\t\"\\"}};
    doc["s"] = section;

    std::string output;
    EXPECT_TRUE(tinycfg::serialize(doc, output).ok());
    EXPECT_NE(output.find(R"("line\n\t\"\\")"), std::string::npos);
}
