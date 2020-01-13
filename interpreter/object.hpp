//
// Created by ayles on 1/10/20.
//

#ifndef NLANG_OBJECT_HPP
#define NLANG_OBJECT_HPP

#include <utils/defs.hpp>
#include <utils/traits.hpp>

#include <string>
#include <string_view>
#include <type_traits>
#include <memory>
#include <utility>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <atomic>


namespace nlang {

class Null;
class Bool;
class Number;
class String;
class Function;
class NLangObject;

class Object {
public:
    enum class Type : uint8_t {
        THE_NULL,
        BOOL,
        NUMBER,
        STRING,
        FUNCTION,
        OBJECT
    };

    template<typename T>
    [[nodiscard]] NLANG_FORCE_INLINE bool Is() const;

    template<typename T>
    [[nodiscard]] NLANG_FORCE_INLINE const T& As() const;

    template<typename T>
    [[nodiscard]] NLANG_FORCE_INLINE T& As();

    [[nodiscard]]
    NLANG_FORCE_INLINE Type GetType() const {
        return type;
    }

    [[nodiscard]]
    NLANG_FORCE_INLINE bool IsTruthy() const {
        return meta & 1u;
    }

    [[nodiscard]]
    NLANG_FORCE_INLINE bool IsFalsy() const {
        return !IsTruthy();
    }

    [[nodiscard]]
    NLANG_FORCE_INLINE
    std::string ToString() const;

protected:
    explicit Object(Type type) : type(type), meta(0u) {}

    const Type type;
    // Used for store type-dependant info such as flags for truthiness
    // 7
    // 6
    // 5
    // 4
    // 3
    // 2
    // 1 - no hash collision bit on strings
    // 0 - truthiness
    std::atomic_uint8_t meta;
};

class Null : public Object {
private:
    friend class ObjectManager;

    Null() : Object(Object::Type::THE_NULL) {}
};

class Bool : public Object {
private:
    friend class ObjectManager;

    explicit Bool(bool flag = false) : Object(Object::Type::BOOL) {
        meta |= (uint8_t)flag;
    }
};

class Number : public Object {
public:
    [[nodiscard]] NLANG_FORCE_INLINE operator double() const {
        return number;
    }

    [[nodiscard]] NLANG_FORCE_INLINE double Get() const {
        return number;
    }

private:
    friend class ObjectManager;

    explicit Number(double number = 0.0)
        : Object(Object::Type::NUMBER)
        , number(number)
    {
        meta |= uint8_t(number != 0.0);
    }

    const double number;
};

class StringKey;

class String : public Object, public std::string {
public:
    NLANG_FORCE_INLINE bool operator==(const String& other) const {
        // No hash collisions in runtime
        if (meta & 2u) {
            return hash == other.hash;
        }
        // Fallback behaviour
        return (hash == other.hash) && (static_cast<const std::string&>(*this) == other);
    }

    NLANG_FORCE_INLINE bool operator!=(const String& other) const {
        return !(*this == other);
    }

private:
    friend class ObjectManager;
    friend class std::hash<String>;
    friend class std::hash<StringKey>;

    explicit String(const std::string_view& sv)
        : Object(Object::Type::STRING)
        , std::string(sv.data())
        , hash(std::hash<std::string_view>()(sv))
    {
        meta |= !uint8_t(empty());
    }

    const size_t hash;
};

struct StringKey {
    union {
        const String* string;
        const std::string_view* string_view;
    };
    bool is_string;

    NLANG_FORCE_INLINE StringKey(const String& string)
        : string(&string)
        , is_string(true)
    {

    }

    NLANG_FORCE_INLINE StringKey(const std::string_view& string_view)
        : string_view(&string_view)
        , is_string(false)
    {

    }

    NLANG_FORCE_INLINE StringKey(const StringKey& other)
        : is_string(other.is_string)
    {
        if (is_string) {
            string = other.string;
        } else {
            string_view = other.string_view;
        }
    }

    NLANG_FORCE_INLINE StringKey& operator=(const StringKey& other) {
        is_string = other.is_string;
        if (is_string) {
            string = other.string;
        } else {
            string_view = other.string_view;
        }
        return *this;
    }

    NLANG_FORCE_INLINE bool operator==(const StringKey& other) {
        if (is_string) {
            if (other.is_string) {
                return *string == *other.string;
            } else {
                return *string == *other.string_view;
            }
        } else {
            if (other.is_string) {
                return *string_view == *other.string;
            } else {
                return *string_view == *other.string_view;
            }
        }
    }

    NLANG_FORCE_INLINE bool operator<(const StringKey& other) {
        if (is_string) {
            if (other.is_string) {
                return *string < *other.string;
            } else {
                return *string < *other.string_view;
            }
        } else {
            if (other.is_string) {
                return *string_view < *other.string;
            } else {
                return *string_view < *other.string_view;
            }
        }
    }
};

}

namespace std {

template<>
struct hash<nlang::String> {
    NLANG_FORCE_INLINE size_t operator()(const nlang::String& s) const noexcept {
        return s.hash;
    }
};

template<>
struct hash<nlang::StringKey> {
    NLANG_FORCE_INLINE size_t operator()(const nlang::StringKey& s) const noexcept {
        if (s.is_string) {
            return s.string->hash;
        } else {
            return std::hash<std::string_view>()(*s.string_view);
        }
    }
};

}

namespace nlang {

class Function : public Object {
public:
    [[nodiscard]] NLANG_FORCE_INLINE const std::string& GetName() const {
        return name;
    }

protected:
    friend class ObjectManager;

    NLANG_FORCE_INLINE Function(const std::string& name = "<anonymous>")
        : Object(Object::Type::FUNCTION)
        , name(name)
    {
        meta |= 1u;
    }

    const std::string name;
};

class NLangObject : public Object {

};

class ObjectManager {
public:
    NLANG_FORCE_INLINE ObjectManager()
        : null_instance(new Null)
        , true_instance(new Bool(true))
        , false_instance(new Bool(false))
    {

    }

    [[nodiscard]] NLANG_FORCE_INLINE std::shared_ptr<const Null> GetNull() const {
        return null_instance;
    }

    [[nodiscard]] NLANG_FORCE_INLINE std::shared_ptr<const Bool> GetBool(bool flag = false) const {
        return flag ? true_instance : false_instance;
    }

    [[nodiscard]] NLANG_FORCE_INLINE std::shared_ptr<const Number> GetNumber(double number = 0.0) const {
        return std::shared_ptr<Number>(new Number(number));
    }

    [[nodiscard]] NLANG_FORCE_INLINE std::shared_ptr<const String> GetString(const std::string_view& sv = "") const {
        auto it = strings_cache.find(sv);
        // cache hit
        if (it != strings_cache.end() && *it->second == sv) {
            return it->second;
        }
        // cache miss or hash overlap
        std::shared_ptr<String> s(new String(sv));
        strings_cache[*s] = s;
        return s;
    }

private:
    const std::shared_ptr<const Null> null_instance;
    const std::shared_ptr<const Bool> true_instance;
    const std::shared_ptr<const Bool> false_instance;
    mutable std::unordered_map<StringKey, std::shared_ptr<const String>> strings_cache;
};

template<typename T>
NLANG_FORCE_INLINE bool Object::Is() const {
    if constexpr (std::is_same_v<T, Null>) {
        return type == Type::THE_NULL;
    } else if constexpr (std::is_same_v<T, Bool>) {
        return type == Type::BOOL;
    } else if constexpr (std::is_same_v<T, Number>) {
        return type == Type::NUMBER;
    } else if constexpr (std::is_same_v<T, String>) {
        return type == Type::STRING;
    } else if constexpr (std::is_same_v<T, Function>) {
        return type == Type::FUNCTION;
    } else if constexpr (std::is_same_v<T, NLangObject>) {
        return type == Type::OBJECT;
    } else {
        static_assert(dependent_true_v<T>, "T is not supported");
    }
}

template<typename T>
NLANG_FORCE_INLINE const T& Object::As() const {
    if constexpr (std::is_same_v<T, Null>) {
        return *static_cast<const Null*>(this);
    } else if constexpr (std::is_same_v<T, Bool>) {
        return *static_cast<const Bool*>(this);
    } else if constexpr (std::is_same_v<T, Number>) {
        return *static_cast<const Number*>(this);
    } else if constexpr (std::is_same_v<T, String>) {
        return *static_cast<const String*>(this);
    } else if constexpr (std::is_same_v<T, Function>) {
        return *static_cast<const Function*>(this);
    } else if constexpr (std::is_same_v<T, NLangObject>) {
        return *static_cast<const NLangObject*>(this);
    } else {
        static_assert(dependent_true_v<T>, "T is not supported");
    }
}

template<typename T>
NLANG_FORCE_INLINE T& Object::As() {
    return const_cast<T&>(const_cast<const Object*>(this)->As<T>());
}

NLANG_FORCE_INLINE std::string Object::ToString() const {
    switch (type) {
        case Object::Type::BOOL:
            return IsTruthy() ? "true" : "false";

        case Object::Type::THE_NULL:
            return "null";

        case Object::Type::NUMBER: {
            std::stringstream s;
            s << std::setprecision(std::numeric_limits<double>::digits10) << As<Number>().Get();
            return s.str();
        }

        case Object::Type::STRING:
            return static_cast<const std::string&>(As<String>());

        case Object::Type::FUNCTION:
            return "[function \"" + As<Function>().GetName() + "\"]";

        case Object::Type::OBJECT:
            return "[object]";
    }
}

}

#endif //NLANG_OBJECT_HPP
