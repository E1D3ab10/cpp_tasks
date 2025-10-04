#include <concepts>

template <typename T>
concept CopyListInitiaslization = requires { T{}; };

struct base_tuple {};

template <typename... Args>
class Tuple;

template <size_t N>
struct element_index {};

template <>
class Tuple<> {
public:
    constexpr Tuple() = default;
    template <size_t N, typename... Other>
    constexpr Tuple(element_index<N>, Other&&...) : Tuple() {}

    constexpr auto operator<=>(const Tuple<>&) const {
        return std::strong_ordering::equal;
    }

    bool operator==(const Tuple<>&) const { return true; }
};

struct tuple_cat_tag {};

template <size_t N, typename... Args, typename... Other>
auto helper_get(Tuple<Args...>&& first, Other&&... other) {
    if constexpr (sizeof...(Args) > N) {
        return get<N>(std::forward<decltype(first)>(first));
    } else {
        return helper_get<N - sizeof...(Args)>(std::forward<Other>(other)...);
    }
}

template <size_t N, typename... Args, typename... Other>
auto helper_get(const Tuple<Args...>& first, Other&&... other) {
    if constexpr (sizeof...(Args) > N) {
        return get<N>(std::forward<decltype(first)>(first));
    } else {
        return helper_get<N - sizeof...(Args)>(std::forward<Other>(other)...);
    }
}
template <size_t N, typename... Args, typename... Other>
auto helper_get(Tuple<Args...>& first, Other&&... other) {
    if constexpr (sizeof...(Args) > N) {
        return get<N>(std::forward<decltype(first)>(first));
    } else {
        return helper_get<N - sizeof...(Args)>(std::forward<Other>(other)...);
    }
}

template <size_t N, typename Tpl, typename Head, typename... Tail>
constexpr bool is_constructible_all_index =
        std::is_constructible_v<Head, decltype(get<N>(std::forward<Tpl>(
                                              std::declval<Tpl>())))> &&
        is_constructible_all_index<N + 1, Tpl, Tail...>;

template <size_t N, typename Tpl, typename Head>
constexpr bool is_constructible_all_index<N, Tpl, Head> =
        std::is_constructible_v<Head, decltype(get<N>(
                                              std::forward<Tpl>(std::declval<Tpl>())))>;

template <size_t N, typename Tpl, typename Head, typename... Tail>
constexpr bool is_convertible_all_index =
        std::is_convertible_v<
                decltype(get<N>(std::forward<Tpl>(std::declval<Tpl>()))), Head> &&
        is_convertible_all_index<N + 1, Tpl, Tail...>;

template <size_t N, typename Tpl, typename Head>
constexpr bool is_convertible_all_index<N, Tpl, Head> = std::is_convertible_v<
        decltype(get<N>(std::forward<Tpl>(std::declval<Tpl>()))), Head>;

template <size_t N, typename... Args>
auto& get(Tuple<Args...>& t) {
    if constexpr (N == 0) {
        return t.head;
    } else {
        return get<N - 1>(t.tail);
    }
}

template <size_t N, typename... Args>
auto&& get(Tuple<Args...>&& t) {
    if constexpr (N == 0) {
        return std::forward<decltype(t.head)>(t.head);
    } else {
        return get<N - 1>(std::move(t.tail));
    }
}

template <typename Head, typename... Tail>
class Tuple<Head, Tail...> : base_tuple {
private:
    Head head;
    [[no_unique_address]] Tuple<Tail...> tail;

    template <typename... Args>
    friend class Tuple;

    template <size_t N, typename... Args>
    friend auto& get(Tuple<Args...>& t);

    template <size_t N, typename... Args>
    friend auto&& get(Tuple<Args...>&& t);
    template <typename T>
        requires(!(sizeof...(Tail) == 0 && !std::is_same_v<T, Head>))
    friend auto& get(Tuple<Head, Tail...>& tpl) {
        if constexpr (std::is_same_v<T, Head>) {
            return tpl.head;
        } else {
            return get<T>(tpl.tail);
        }
    }

    template <typename T>
        requires(!(sizeof...(Tail) == 0 && !std::is_same_v<T, Head>)) &&
                (std::is_same_v<Head, T> || (std::is_same_v<T, Tail> && ...))

    friend const T& get(const Tuple<Head, Tail...>& tpl) {
        if constexpr (std::is_same_v<T, Head>) {
            return tpl.head;
        } else {
            return get<T>(tpl.tail);
        }
    }

    template <size_t N>
    friend const auto& get(const Tuple<Head, Tail...>& t) {
        if constexpr (N == 0) {
            return t.head;
        } else {
            return get<N - 1>(t.tail);
        }
    }

    template <typename T>
        requires(!(sizeof...(Tail) == 0 && !std::is_same_v<T, Head>)) &&
                (std::is_same_v<T, Head> || (std::is_same_v<T, Tail> || ...))

    friend T&& get(Tuple<Head, Tail...>&& tpl) {
        if constexpr (std::is_same_v<T, Head>) {
            return std::move(tpl.head);
        } else {
            return get<T>(std::move(tpl.tail));
        }
    }

    template <typename T>
        requires(!(sizeof...(Tail) == 0 && !std::is_same_v<T, Head>)) &&
                (std::is_same_v<T, Head> || (std::is_same_v<T, Tail> || ...))

    friend const T&& get(const Tuple<Head, Tail...>&& tpl) {
        if constexpr (std::is_same_v<T, Head>) {
            return std::move(tpl.head);
        } else {
            return get<T>(std::move(tpl.tail));
        }
    }
    constexpr size_t sz() { return 1 + sizeof...(Tail); }

    friend auto tupleCat(auto&&... tpls);

    template <size_t N, typename First, typename... Other>
    friend auto helper_get(First&& first, Other&&... other);

public:
    template <size_t N, typename First, typename Second, typename... Other>
    Tuple(element_index<N>, First&& first, Second&& second, Other&&... other)
        : head(helper_get<N>(std::forward<First>(first),
                             std::forward<Second>(second),
                             std::forward<Other>(other)...)),
          tail(element_index<N + 1>(), std::forward<First>(first),
               std::forward<Second>(second), std::forward<Other>(other)...) {}

    template <size_t N>
    Tuple(element_index<N>) : Tuple() {}

    explicit(!(CopyListInitiaslization<Head> &&
               (CopyListInitiaslization<Tail> && ...))) Tuple()
        requires(std::is_default_constructible_v<Head> &&
                 (std::is_default_constructible_v<Tail> && ...))
        : head(), tail() {}

    explicit(!(std::is_convertible_v<const Head&, Head> &&
               (std::is_convertible_v<const Tail&, Tail> && ...)))
            Tuple(const Head& head, const Tail&... tail)
        requires(std::is_copy_constructible_v<Head> &&
                 (std::is_copy_constructible_v<Tail> && ...))
        : head(head), tail(tail...) {}

    template <typename Head2, typename... Tail2>
    explicit(!(std::is_convertible_v<Head2, Head> &&
               (std::is_convertible_v<Tail2, Tail> && ...)))
            Tuple(Head2&& head2, Tail2&&... tail2)
        requires(sizeof...(Tail2) == sizeof...(Tail) &&
                 (std::is_constructible_v<Head, Head2> &&
                  (std::is_constructible_v<Tail, Tail2> && ...)))
        : head(std::forward<Head2>(head2)), tail(std::forward<Tail2>(tail2)...) {}

    template <typename Head2, typename... Tail2>
    Tuple(const Tuple<Head2, Tail2...>& tpl)
        requires(sizeof...(Tail) == sizeof...(Tail2) &&
                 is_convertible_all_index<0, decltype(tpl), Head, Tail...> &&
                 is_constructible_all_index<0, decltype(tpl), Head, Tail...> &&
                 (sizeof...(Tail2) != 0 ||
                  !(std::is_convertible_v<decltype(tpl), Head> ||
                    std::is_constructible_v<Head, decltype(tpl)> ||
                    std::is_same_v<Head, Head2>)))
        : head(get<0>(std::forward<decltype(tpl)>(tpl))), tail(tpl.tail) {}

    template <typename Head2, typename... Tail2>
    explicit Tuple(const Tuple<Head2, Tail2...>& tpl)
        requires(sizeof...(Tail) == sizeof...(Tail2) &&
                 !is_convertible_all_index<0, decltype(tpl), Head, Tail...> &&
                 is_constructible_all_index<0, decltype(tpl), Head, Tail...> &&
                 (sizeof...(Tail2) != 0 ||
                  !(std::is_convertible_v<decltype(tpl), Head> ||
                    std::is_constructible_v<Head, decltype(tpl)> ||
                    std::is_same_v<Head, Head2>)))
        : head(get<0>(std::forward<decltype(tpl)>(tpl))), tail(tpl.tail) {}

    template <typename Head2, typename... Tail2>
    Tuple(Tuple<Head2, Tail2...>&& tpl)
        requires(sizeof...(Tail) == sizeof...(Tail2) &&
                 is_convertible_all_index<0, decltype(tpl), Head, Tail...> &&
                 is_constructible_all_index<0, decltype(tpl), Head, Tail...> &&
                 (sizeof...(Tail2) != 0 ||
                  !(std::is_convertible_v<decltype(tpl), Head> ||
                    std::is_constructible_v<Head, decltype(tpl)> ||
                    std::is_same_v<Head, Head2>)))
        : head(get<0>(std::forward<decltype(tpl)>(tpl))),
          tail(std::move(tpl.tail)) {}

    template <typename Head2, typename... Tail2>
    explicit Tuple(Tuple<Head2, Tail2...>&& tpl)
        requires(sizeof...(Tail) == sizeof...(Tail2) &&
                 !is_convertible_all_index<0, decltype(tpl), Head, Tail...> &&
                 is_constructible_all_index<0, decltype(tpl), Head, Tail...> &&
                 (sizeof...(Tail2) != 0 ||
                  !(std::is_convertible_v<decltype(tpl), Head> ||
                    std::is_constructible_v<Head, decltype(tpl)> ||
                    std::is_same_v<Head, Head2>)))
        : head(get<0>(std::forward<decltype(tpl)>(tpl))),
          tail(std::move(tpl.tail)) {}

    template <typename T1, typename T2>
    Tuple(const std::pair<T1, T2>& pair) : head(pair.first), tail(pair.second) {}

    template <typename T1, typename T2>
    Tuple(std::pair<T1, T2>&& pair)
        : head(std::move(pair.first)), tail(std::move(pair.second)) {}

    Tuple(const Tuple& other)
        requires(std::is_copy_constructible_v<Head> &&
                 (std::is_copy_constructible_v<Tail> && ...))
    = default;

    Tuple(Tuple&& other)
        : head(std::move(other.head)), tail(std::move(other.tail)) {}

    Tuple& operator=(const Tuple& other)
        requires(std::is_copy_assignable_v<Head> &&
                 (std::is_copy_assignable_v<Tail> && ...))
    {
        head = other.head;
        tail = other.tail;
        return *this;
    }

    Tuple& operator=(Tuple&& other) noexcept
        requires(std::is_move_assignable_v<Head> &&
                 (std::is_move_assignable_v<Tail> && ...))
    {
        head = get<0>(std::forward<decltype(other)>(other));
        tail = std::move(other.tail);
        return *this;
    }

    template <typename Head2, typename... Tail2>
    Tuple& operator=(const Tuple<Head2, Tail2...>& other)
        requires(sizeof...(Tail2) == sizeof...(Tail) &&
                 std::is_assignable_v<Head&, const Head2&> &&
                 (std::is_assignable_v<Tail&, const Tail2&> && ...))
    {
        head = other.head;
        tail = other.tail;
        return *this;
    }

    template <typename Head2, typename... Tail2>
    Tuple& operator=(Tuple<Head2, Tail2...>&& other)
        requires(sizeof...(Tail2) == sizeof...(Tail) &&
                 std::is_assignable_v<Head&, Head2> &&
                 (std::is_assignable_v<Tail&, Tail2> && ...))
    {
        head = std::forward<Head2>(get<0>(other));
        tail = std::move(other.tail);
        return *this;
    }

    template <typename T1, typename T2>
    Tuple& operator=(const std::pair<T1, T2>& pair)
        requires(sizeof...(Tail) == 1)
    {
        head = pair.first;
        tail = pair.second;
        return *this;
    }

    template <typename T1, typename T2>
    Tuple& operator=(std::pair<T1, T2>&& pair)
        requires(sizeof...(Tail) == 1)
    {
        head = std::move(pair.first);
        tail = std::move(pair.second);
        return *this;
    }

    template <typename... UTypes>
    constexpr auto operator<=>(const Tuple<UTypes...>& other) const {
        if constexpr (sizeof...(UTypes) == 0) {
            return std::strong_ordering::greater;
        }
        if (head == other.head) {
            return (tail <=> other.tail);
        }
        return head <=> other.head;
    }

    template <typename... UTypes>
    bool operator==(const Tuple<UTypes...>& other) const {
        return head == other.head && tail == other.tail;
    }
};

template <typename Tuple1, typename Tuple2>
struct tuple_concat;

template <typename... Ts1, typename... Ts2>
struct tuple_concat<Tuple<Ts1...>, Tuple<Ts2...>> {
    using type = Tuple<Ts1..., Ts2...>;
};

template <typename... Tuples>
struct tuple_cat_impl;

template <>
struct tuple_cat_impl<> {
    using type = Tuple<>;
};

template <typename Tuple>
struct tuple_cat_impl<Tuple> {
    using type = Tuple;
};

template <typename First, typename Second, typename... Rest>
struct tuple_cat_impl<First, Second, Rest...> {
    using type = typename tuple_cat_impl<
            typename tuple_concat<typename std::decay<First>::type,
                                  typename std::decay<Second>::type>::type,
            typename std::decay<Rest>::type...>::type;
};

template <typename... Tuples>
using tuple_cat_t = typename tuple_cat_impl<Tuples...>::type;

auto tupleCat(auto&&... tpls) {
    using T = tuple_cat_t<decltype(tpls)...>;
    return T(element_index<0>(), std::forward<decltype(tpls)>(tpls)...);
}

template <typename T1, typename T2>
Tuple(const std::pair<T1, T2>&) -> Tuple<T1, T2>;
template <typename T1, typename T2>
Tuple(std::pair<T1, T2>&&) -> Tuple<T1, T2>;

template <typename... Types>
auto makeTuple(Types&&... args) -> Tuple<std::decay_t<Types>...> {
    return {std::forward<Types>(args)...};
}

template <typename... Types>
auto tie(Types&... args) -> Tuple<Types&...> {
    return {args...};
}

template <typename... Types>
auto forwardAsTuple(Types&&... args) -> Tuple<Types&&...> {
    return {std::forward<Types>(args)...};
}
