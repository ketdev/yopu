#pragma once
#include <memory>

namespace std {
    template <typename _Ptr, void _Dx(_Ptr*)>
    struct _del {
        void operator()(_Ptr* ptr) const noexcept { _Dx(ptr); }
    };

    template <typename _Ty, void _Dx(_Ty*)>
    using clean_ptr = std::unique_ptr<_Ty, _del<_Ty, _Dx> >;

    template <typename _Ty, void _Dx(void*)>
    using clean_vptr = std::unique_ptr<void, _del<void, _Dx> >;

}  // namespace std