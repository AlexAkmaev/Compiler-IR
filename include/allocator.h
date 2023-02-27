#ifndef COMPILER_ALLOCATOR_H
#define COMPILER_ALLOCATOR_H

#include <vector>
#include <algorithm>
#include <memory>
#include <tuple>
#include <cassert>

namespace compiler::storage {

template<class C>
static constexpr bool HasGetIdMethod(...) { return false; }

template<class C>
static constexpr bool HasGetIdMethod(int, decltype((std::declval<C>().GetId(float()))) * = 0) { return true; }

template<class C>
class Storage final {
public:
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
            for (auto &elem: holder_) {
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

template<class C>
class PoolStorage final {
public:
    template<class... T, std::enable_if_t<(std::is_same_v<T, C> && ...), bool> = true>
    void AddPool(T &&... classes) {
        holder_.template emplace_back(std::vector<C>{std::forward<C>(classes)...});
        std::vector<C *> pointers_pool;
        std::transform(holder_.back().begin(), holder_.back().end(), std::back_inserter(pointers_pool),
                       [](C &cls) { return &cls; });
        pointers_holder_.template emplace_back(pointers_pool);
    }

    void AddPool(std::vector<C> &&classes) {
        holder_.template emplace_back(std::forward<std::vector<C>>(classes));
        std::vector<C *> pointers_pool;
        std::transform(holder_.back().begin(), holder_.back().end(), std::back_inserter(pointers_pool),
                       [](C &cls) { return &cls; });
        pointers_holder_.template emplace_back(pointers_pool);
    }

    [[nodiscard]] std::vector<C *> GetFrontPointersPool() const {
        return pointers_holder_.front();
    }

    [[nodiscard]] std::vector<C *> GetBackPointersPool() {
        return pointers_holder_.back();
    }

    template<size_t N>
    [[nodiscard]] auto GetFrontPointersPoolAsArray() {
        const std::vector<C *> &vec = pointers_holder_.front();
        assert(N == vec.size());
        std::array<C *, N> arr;
        std::copy_n(vec.begin(), N, arr.begin());
        return arr;
    }

    template<size_t N>
    [[nodiscard]] auto GetBackPointersPoolAsArray() {
        const std::vector<C *> &vec = pointers_holder_.back();
        assert(N == vec.size());
        std::array<C *, N> arr;
        std::copy_n(vec.begin(), N, arr.begin());
        return arr;
    }

private:
    std::vector<std::vector<C>> holder_;
    std::vector<std::vector<C *>> pointers_holder_;  // copy of holder_ but transformed to pointers
};

template<class... C>
class Allocator final {
public:
    template<class Cls, std::enable_if_t<(std::is_same_v<Cls, C> || ...), bool> = true>
    Cls *New(Cls &&cls) {
        Storage<Cls> &stg = std::get<Storage<Cls>>(storages_);
        stg.AddElem(std::forward<Cls>(cls));
        return stg.GetBackPointer();
    }

    template<class T, class... Cls,
            std::enable_if_t<(std::is_same_v<T, Cls> && ...), bool> = true,
            std::enable_if_t<(std::is_same_v<T, C> || ...), bool> = true>
    std::vector<T *> NewPool(Cls &&... cls) {
        PoolStorage<T> &stg = std::get<PoolStorage<T>>(pool_storages_);
        stg.AddPool(std::forward<T>(cls)...);
        return stg.GetBackPointersPool();
    }

    template<class T, class... Cls,
            std::enable_if_t<(std::is_same_v<T, Cls> && ...), bool> = true,
            std::enable_if_t<(std::is_same_v<T, C> || ...), bool> = true>
    std::array<T *, sizeof...(Cls)> NewPoolAsArray(Cls &&... cls) {
        PoolStorage<T> &stg = std::get<PoolStorage<T>>(pool_storages_);
        stg.AddPool(std::forward<Cls>(cls)...);
        return stg.template GetBackPointersPoolAsArray<sizeof...(Cls)>();
    }

private:
    std::tuple<Storage<C>...> storages_ = std::make_tuple(Storage<C>{}...);
    std::tuple<PoolStorage<C>...> pool_storages_ = std::make_tuple(PoolStorage<C>{}...);
};

}  // namespace compiler::storage

#endif //COMPILER_ALLOCATOR_H
