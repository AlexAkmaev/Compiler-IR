#ifndef COMPILER_STORAGE_H
#define COMPILER_STORAGE_H

#include <vector>

#include "basic_block.h"

namespace compiler {

template <class C>
static constexpr bool HasGetIdMethod(...) { return false; }

template <class C>
static constexpr bool HasGetIdMethod(int, decltype((std::declval<C>().GetId(float() )))* = 0) { return true; }

template<class C>
class Storage final {
public:
    Storage() = default;
    
    void AddElem(C &&elem) { 
        holder_.push_back(std::forward<C>(elem));
    }

    [[nodiscard]] C *GetFrontPointer() const {
        return &holder_.front();
    }
    
    [[nodiscard]] C *GetBackPointer() {
        return &holder_.back();
    }
    
    [[nodiscard]] C *GetPointerById(size_t id) const {
        if constexpr (HasGetIdMethod<C>(0)) {
            for (auto &elem : holder_) {
                if (elem.GetId() == id) {
                    return &elem;
                }
            }
        }
        return nullptr;
    }
    
private:
    std::vector<C> holder_;
};

Storage<BasicBlock> bbs_storage;

}  // namespace compiler 

#endif //COMPILER_STORAGE_H
