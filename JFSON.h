//
// Created by fl on 17-11-18.
//

#ifndef JFSON_JFSON_H
#define JFSON_JFSON_H

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <initializer_list>

namespace JFSON{

enum JsonParse {
    STANDARD, COMMENTS
};

class JsonValue;

class Json final {
public:
    //Json Types
    enum Type {
        NUL, NUMBER, BOOL, STRING, ARRAY, OBJECT
    };

    //Arrays and objects typedefs
    typedef std::vector<Json> array;
    typedef std::map<std::string, Json> object;

    //constructors for various types of Json values
    Json() noexcept;                // NUL
    Json(std::nullptr_t) noexcept;  // NUL
    Json(double value);             // NUMBER
    Json(int value);                // NUMBER
    Json(bool value);               // BOOL
    Json(const std::string &value); // STRING
    Json(std::string &&value);      // STRING
    Json(const char * value);       // STRING
    Json(const array &values);      // ARRAY
    Json(array &&values);           // ARRAY
    Json(const object &value);      // OBJECT
    Json(object &&value);           // OBJECT

    //implicit constructor: anything with a to_string() function.
    template <class T, class = decltype(&T::to_json)>
    Json(const T &t) : Json(t.to_json()) {}

    //implicit constructor: map-like objects (std::map, std::unordered_map, etc.)
    template <class M, typename std::enable_if<
        std::is_constructible<std::string, decltype(std::declval<M>().begin()->first)>::value
        && std::is_constructible<Json, decltype(std::declval<M>().begin()->first)>::value,
            int>::type = 0>
    Json(const M & m) : Json(object(m.begin(), m.end())) {}

    //implicit constructor: vector-like objects (std::list, std::vector, std::set, etc.)
    template <class V, typename std::enable_if<
        std::is_constructible<Json, decltype(*std::declval<V>().begin())>::value,
            int>::type = 0>
    Json(const V & v) : Json(array(v.begin(), v.end())) {}

    // This prevents Json(some_pointer) from accidentally producing a bool. Use
    // Json(bool(some_pointer)) if that behavior is desired.
    Json(void *) = delete;

    //Accessors
    Type type() const;

    bool is_null() const { return type() == NUL; }
    bool is_number() const { return type() == NUMBER; }
    bool is_bool() const { return type() == BOOL; }
    bool is_string() const { return type() == STRING; }
    bool is_array() const { return type() == ARRAY; }
    bool is_object() const { return type() == OBJECT; }

    // Return the enclosed value if this is a number, 0 otherwise. Note that JFSON doesn't
    // distinguish between integer and non-integer numbers, which means that number_value()
    // and int_value() can both be applied to a NUMBER-typed object.
    double number_value() const;
    int int_value() const;

    // Return the enclosed value if this is a boolean, false otherwise.
    bool bool_value() const;

    // Return the enclosed string if this is a stringt, "" otherwise.
    const std::string &string_value() const;

    // Return the enclosed std::vector if this is an array, or an empty vector otherwise
    const array &array_items() const;

    // Return the enclosed std::map if this is an object, or an empty map otherwise.
    const object &object_items() const;

    // Return a reference to arr[i] if this is an array, Json() otherwise.
    const Json & operator[](size_t i) const;
    // Return a reference to obj[key] if this is an object, Json() otherwise.
    const Json &operator[](const std::string &key) const;

    // Serialize.
    void dump(std::string &out) const;
    std::string dump() const {
        std::string out;
        dump(out);
        return out;
    }

    // Parse. If parse fails, return Json() and assign an error message to err.
    static Json parse(const std::string &in, std::string & err, JsonParse strategy = JsonParse::STANDARD);
    static Json parse(const char * in, std::string & err, JsonParse strategy = JsonParse::STANDARD) {
        if (in) {
            return parse(std::string(in), err, strategy);
        } else {
            err = "null input";
            return nullptr;
        }
    }

    // Parse multiple objects, concatenated or separated by whitespace
    static std::vector<Json> parse_multi(
            const std::string& in,
            std::string::size_type  & parser_stop_pos,
            std::string & err,
            JsonParse strategy = JsonParse ::STANDARD
    );

    static inline std::vector<Json> parse_multi(
            const std::string & in,
            std::string & err,
            JsonParse strategy = JsonParse::STANDARD) {
        std::string::size_type parser_stop_pos;
        return parse_multi(in, parser_stop_pos, err, strategy);
    }

    bool operator== (const Json &rhs) const;
    bool operator<  (const Json &rhs) const;
    bool operator!= (const Json &rhs) const { return !(*this == rhs); }
    bool operator<= (const Json &rhs) const { return !(rhs < *this); }
    bool operator>  (const Json &rhs) const { return (rhs < *this); }
    bool operator>= (const Json &rhs) const { return !(*this < rhs); }

    /* has_shape(types, err)
     *
     * Return true if this is a Json object and, for each item in types, has a field
     * of the given type. If not, return false and set err to a descriptive message.
     */
    typedef std::initializer_list<std::pair<std::string, Type>> shape;
    bool has_shape(const shape & types, std::string & err) const;

private:
    std::shared_ptr<JsonValue> m_ptr;

};

// Internal class hierarchy - JsonValue objects are not exposed to users of this API.
class JsonValue {
protected:
    friend class Json;
    friend class JsonInt;
    friend class JsonDouble;
    virtual Json::Type type() const = 0;
    virtual bool equals(const JsonValue * other) const = 0;
    virtual bool less(const JsonValue * other) const = 0;
    virtual void dump(std::string &out) const = 0;
    virtual double number_value() const;
    virtual int int_value() const;
    virtual bool bool_value() const;
    virtual const std::string &string_value() const;
    virtual const Json::array &array_items() const;
    virtual const Json &operator[](size_t i) const;
    virtual const Json::object &object_items() const;
    virtual const Json &operator[](const std::string &key) const;
    virtual ~JsonValue() {}
};

} // namespace JFSON

#endif //JFSON_JFSON_H
