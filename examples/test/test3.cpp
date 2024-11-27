#include <iostream>
#include <type_traits>
#include <utility>

class A {
  public:
    virtual void print() const { std::cout << "This is class A" << std::endl; }
    virtual ~A() = default;
};

class B : public A {
  public:
    int x;
    B(int x) : x(x) {}
    void print() const override { std::cout << "This is class B with x = " << x << std::endl; }
};

class C : public A {
  public:
    int y;
    C(int y) : y(y) {}
    void print() const override { std::cout << "This is class C with y = " << y << std::endl; }
};

class Factory {
  public:
    template <typename T, typename... Args>
        requires std::derived_from<T, A>
    static T create(Args &&...args) {
        if constexpr (std::same_as<T, B>) {
            std::cout << "Creating an instance of B" << std::endl;
            auto processed_args = process_args_B(std::forward<Args>(args)...);
            return T(std::get<0>(processed_args));
        } else if constexpr (std::same_as<T, C>) {
            std::cout << "Creating an instance of C" << std::endl;
            auto processed_args = process_args_C(std::forward<Args>(args)...);
            return T(std::get<0>(processed_args));
        } else {
            static_assert(std::false_type::value, "Unsupported type");
        }
    }

  private:
    template <typename... Args> static auto process_args_B(Args &&...args) {
        return std::make_tuple((std::forward<Args>(args))...);
    }

    template <typename... Args> static auto process_args_C(Args &&...args) {
        return std::make_tuple((std::forward<Args>(args))...);
    }
};

int main() {
    auto b = Factory::create<B>(5);
    auto c = Factory::create<C>(10);

    // Use the objects
    b.print();   // Output: This is class B with x = 15
    c.print();   // Output: This is class C with y = 5

    return 0;
}
