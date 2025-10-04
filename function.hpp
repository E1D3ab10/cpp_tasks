#include <functional>
#include <iostream>
#include <memory>
enum _Manager_operation { __get_type_info, __clone_functor, __destroy_functor };

template <bool IsMoveOnly, typename T>
class BaseFunction;

template <bool IsMoveOnly, typename Ret, typename... Args>
class BaseFunction<IsMoveOnly, Ret(Args...)> {

protected:

    using invoke_ptr_t = Ret (*)(void*, Args...);
    using manager_ptr_t = void* (*)(_Manager_operation, void*, void*);

    static const size_t BUFFER_SIZE = 16;
    void* fptr = nullptr;
    alignas(std::max_align_t) char buffer[BUFFER_SIZE];
    manager_ptr_t manager_ptr = nullptr;
    invoke_ptr_t invoke_ptr = nullptr;

    template <typename Functor>
    struct Manager {

        static void destroyer(Functor* fptr) {

            if constexpr (sizeof(Functor) > BUFFER_SIZE) {
                delete fptr;
            }

            else {
                fptr->~Functor();
            }

        }

        static Functor* copier(Functor* fptr, void* buffer) {

            if constexpr (!IsMoveOnly) {

                if constexpr (sizeof(Functor) >
                              BaseFunction<IsMoveOnly, Ret(Args...)>::BUFFER_SIZE) {
                    return new Functor(*fptr);
                }

                else {
                    return new (buffer) Functor(*fptr);
                }
            }

            return nullptr;

        }

        static Functor* targeter(Functor* fptr) { return fptr; }

        static const std::type_info* type_info(Functor* fptr) {

            if (fptr != nullptr) {
                return &typeid(Functor);
            }

            return &typeid(void);
        }

        static Functor* get_ptr(_Manager_operation op, Functor* fptr, void* buffer) {

            switch (op) {

                case __clone_functor:
                    return copier(fptr, buffer);

                case __destroy_functor:
                    destroyer(fptr);
                    return nullptr;

                case __get_type_info:
                    return const_cast<Functor*>(
                            reinterpret_cast<const Functor*>(type_info(fptr)));

            }

            return nullptr;

        }

    };

public:

    template <typename F>
    static Ret invoker(F* fptr, Args... args) {
        return std::invoke(*fptr, std::forward<Args>(args)...);
    }

    BaseFunction() = default;
    BaseFunction(std::nullptr_t) : BaseFunction() {}

    template <typename F>
        requires(!std::is_same_v<std::remove_cv<F>,
                                 BaseFunction<IsMoveOnly, Ret(Args...)>> &&
                 std::is_convertible_v<std::invoke_result_t<F, Args...>, Ret>)

    BaseFunction(F&& func)
        : manager_ptr(reinterpret_cast<manager_ptr_t>(
                  &Manager<std::decay_t<F>>::get_ptr)),
          invoke_ptr(reinterpret_cast<invoke_ptr_t>(&invoker<std::decay_t<F>>)) {

        if constexpr (sizeof(std::decay_t<F>) > BUFFER_SIZE) {
            fptr = new std::decay_t<F>(std::forward<F>(func));
        }

        else {
            new (buffer) std::decay_t<F>(std::forward<F>(func));
            fptr = buffer;
        }

    }

    BaseFunction(const BaseFunction& other) requires(!IsMoveOnly)
        : fptr(other.manager_ptr(__clone_functor, other.fptr, buffer)),
          manager_ptr(other.manager_ptr),
          invoke_ptr(other.invoke_ptr) {}

    void swap(BaseFunction& other) noexcept {

        if (fptr != buffer && other.fptr != other.buffer) {
            std::swap(fptr, other.fptr);
        }

        else if (fptr != buffer) {
            other.fptr = fptr;
            fptr = buffer;
        }

        else if (other.fptr != other.buffer) {
            fptr = other.fptr;
            other.fptr = other.buffer;
        }

        std::swap(buffer, other.buffer);
        std::swap(invoke_ptr, other.invoke_ptr);
        std::swap(manager_ptr, other.manager_ptr);

    }

    BaseFunction(BaseFunction&& other) { swap(other); }

    BaseFunction& operator=(BaseFunction&& other) noexcept {

        if (this == &other) {
            return *this;
        }

        BaseFunction copy(std::move(other));
        swap(copy);
        return *this;

    }

    BaseFunction& operator=(const BaseFunction& other) requires(!IsMoveOnly) {

        if (this == &other) {
            return *this;
        }

        BaseFunction copy(other);
        swap(copy);
        return *this;

    }

    template <typename F>
    BaseFunction& operator=(F&& func) requires(
                !std::is_same_v<std::decay_t<F>,
                                BaseFunction<IsMoveOnly, Ret(Args...)>> &&
                std::is_convertible_v<std::invoke_result_t<F, Args...>, Ret>) {

        BaseFunction copy(std::forward<F>(func));
        swap(copy);
        return *this;

    }

    Ret operator()(Args... args) const {

        if (fptr == nullptr) {
            throw std::bad_function_call();
        }

        return invoke_ptr(fptr, std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept { return fptr != nullptr; }

    template <typename T>
    T* target() {
        return reinterpret_cast<T*>(fptr);
    }

    template <typename T>
    const T* target() const {
        return reinterpret_cast<const T*>(fptr);
    }

    const std::type_info& target_type() const noexcept {

        return *reinterpret_cast<std::type_info*>(
                manager_ptr(__get_type_info, fptr, const_cast<char*>(buffer)));

    }

    bool operator==(std::nullptr_t) noexcept { return !*this; }

    ~BaseFunction() {

        if (fptr) {

            manager_ptr(__destroy_functor, fptr, buffer);

        }

    }
};

template <typename>
class MoveOnlyFunction;

template <typename Ret, typename... Args>
class MoveOnlyFunction<Ret(Args...)> : public BaseFunction<true, Ret(Args...)> {
    using BaseFunction<true, Ret(Args...)>::BaseFunction;
};

template <typename>
class Function;

template <typename Ret, typename... Args>
class Function<Ret(Args...)> : public BaseFunction<false, Ret(Args...)> {
    using BaseFunction<false, Ret(Args...)>::BaseFunction;
};

template <typename F, typename... Args>
MoveOnlyFunction(F (*)(Args...)) -> MoveOnlyFunction<F(Args...)>;

template <typename F, typename... Args>
Function(F (*)(Args...)) -> Function<F(Args...)>;

template <typename>
struct function_guide_helper {};

template <typename Res, typename Tp, typename... Args>
struct function_guide_helper<Res (Tp::*)(Args...)> {
    using type = Res(Args...);
};

template <typename Res, typename Tp, typename... Args>
struct function_guide_helper<Res (Tp::*)(Args...) const> {
    using type = Res(Args...);
};

template <typename Op>
using function_guide_t = function_guide_helper<Op>::type;

template <typename Fn,
         typename Signature = function_guide_t<decltype(&Fn::operator())>>
Function(Fn) -> Function<Signature>;

template <typename Fn,
         typename Signature = function_guide_t<decltype(&Fn::operator())>>
MoveOnlyFunction(Fn) -> MoveOnlyFunction<Signature>;
