/**
 * @file   function_view.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2017.12.31
 *
 * @brief  Faster generic callable interface (see https://vittorioromeo.info/index/blog/passing_functions_to_functions.html).
 * current version taken from https://github.com/llvm-mirror/llvm/blob/master/include/llvm/ADT/STLExtras.h#L80 (function_ref)
 */

#pragma once

#include <type_traits>
#include <functional>

namespace viscom {

    /// An efficient, type-erasing, non-owning reference to a callable. This is
    /// intended for use as the type of a function parameter that is not used
    /// after the function in question returns.
    ///
    /// This class does not own the callable, so it is not in general safe to store
    /// a function_ref.
    template<typename Fn> class function_view;

    /** @see \ref viscom::function_view< Fn > */
    template<typename Ret, typename ...Params>
    class function_view<Ret(Params...)> {
        Ret(*callback)(intptr_t callable, Params ...params) = nullptr;
        intptr_t callable;

        template<typename Callable>
        static Ret callback_fn(intptr_t callable, Params ...params) {
            return (*reinterpret_cast<Callable*>(callable))(
                std::forward<Params>(params)...);
        }

    public:
        function_view() = default;
        /** Constructor for a function view with a nullptr. */
        function_view(std::nullptr_t) {}

        /**
         *  Constructor for function_view.
         *  @param callable the callable object to be wrapped by the \ref function_view.
         */
        template <typename Callable>
        function_view(Callable &&callable,
            typename std::enable_if<
            !std::is_same<typename std::remove_reference<Callable>::type,
            function_view>::value>::type * = nullptr)
            : callback(callback_fn<typename std::remove_reference<Callable>::type>),
            callable(reinterpret_cast<intptr_t>(&callable)) {}

        /** Calls the function contained within the function view with parameters. */
        Ret operator()(Params ...params) const {
            return callback(callable, std::forward<Params>(params)...);
        }

        /** Checks whether the function_view contains a function. */
        operator bool() const { return callback; }
    };
}
