#include <iostream>
#include <cassert>
#include "JFSON.h"

#define JFSON_TEST_ASSERT(b) assert(b)

using std::string;
using namespace JFSON;

int main() {
    const string simple_test =
            R"({"k1":"v1", "k2":42, "k3":["a",123,true,false,null]})";
    string err;
    const auto json = Json::parse(simple_test, err);
    std::cout << "k1: " << json["k1"].string_value() << "\n";
    std::cout << "k3: " << json["k3"].dump() << "\n";

    for (auto &k : json["k3"].array_items()) {
        std::cout << "    - " << k.dump() << "\n";
    }

    /*
     * Simple comment test
     * */
    string comment_test = R"({
      // comment /* with nested comment */
      "a": 1,
      // comment
      // continued
      "b": "text",
      /* multi
         line
         comment
        // line-comment-inside-multiline-comment
      */
      // and single-line comment
      // and single-line comment /* multiline inside single line */
      "c": [1, 2, 3]
      // and single-line comment at end of object
    })";

    string err_comment;
    auto json_comment = Json::parse(comment_test, err_comment, JsonParse::COMMENTS);
    JFSON_TEST_ASSERT(!json_comment.is_null());
    JFSON_TEST_ASSERT(err_comment.empty());

    return 0;
}