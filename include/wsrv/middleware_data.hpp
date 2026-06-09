#pragma once

#include <typeindex>
#include <any>
#include <unordered_map>
#include <any>

namespace ws {

class MiddlwareDataStorage {
private:
    std::unordered_map<std::type_index, std::any> storage_;
public:
    template <typename T>
    void set(std::shared_ptr<T> ptr) {
        if (!ptr) return;
        // Key against the underlying type T, not the shared_ptr wrapper
        storage_[std::type_index(typeid(T))] = ptr;
    }

    // Return a clean shared_ptr<T> directly. Returns nullptr if missing.
    template <typename T>
    std::shared_ptr<T> get() {
        auto it = storage_.find(std::type_index(typeid(T)));
        if (it == storage_.end()) return nullptr ;
        return std::any_cast<std::shared_ptr<T>>(it->second);
    }

    // Delete an attribute
    template <typename T>
    void remove() {
        storage_.erase(std::type_index(typeid(T)));
    }
};

}