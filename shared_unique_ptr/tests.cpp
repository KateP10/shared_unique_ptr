#include "tests.h"
#include "array.h"
#include <iomanip>
#include <chrono>
#include <memory>  



struct Base { //базовый
    virtual ~Base() {}
    virtual const char* name() const { return "Base"; }
};

struct Derived : Base { //наследник
    const char* name() const override { return "Derived"; }
};

//счетчик живых объектов - чтобы доказать отсутствие утечек
struct Tracked {
    static int alive; // одна переменная на всех (переменная принадлежит классу, а не объекту)
    Tracked() { ++alive; } // конструктор: +1
    ~Tracked() { --alive; } // деструктор: −1
};
int Tracked::alive = 0;

//подтипизация
void test_unq_subtyping() {
    UnqPtr<Derived> d(new Derived());
    UnqPtr<Base>    b(std::move(d));  
    std::cout << "b->name() = " << b->name() << "\n";
}

//подтипизация
void test_shrd_subtyping() {
    ShrdPtr<Derived> d(new Derived());
    ShrdPtr<Base>    b = d;            
    std::cout << "use_count = " << b.use_count() << "\n";
    std::cout << "b->name() = " << b->name() << "\n";
}

//отсутствие утечек
void test_no_leak() {
    Tracked::alive = 0;
    {
        ShrdPtr<Tracked> p1(new Tracked());
        {
            ShrdPtr<Tracked> p2 = p1;
            ShrdPtr<Tracked> p3 = p2;
        }
    }
    std::cout << "alive after scope = " << Tracked::alive << " (ожидается 0)\n";
}

//использование ShrdPtr в контейнере
void test_container() {
    SharedArray<Tracked> arr;
    arr.push_back(ShrdPtr<Tracked>(new Tracked()));
    arr.push_back(ShrdPtr<Tracked>(new Tracked()));
    ShrdPtr<Tracked> copy = arr.get(0);
    std::cout << "copy.use_count() = " << copy.use_count() << "\n";
}

//замер сырых указателей: создаём N объектов, сохраняем в массив, удаляем
//показывает базовую стоимость без умных указателей.
long long bench_raw(size_t N) {
    using clock = std::chrono::high_resolution_clock;

    Tracked::alive = 0;
    auto t0 = clock::now();

    Tracked** arr = new Tracked * [N];
    for (size_t i = 0; i < N; ++i) {
        arr[i] = new Tracked();
    }
    for (size_t i = 0; i < N; ++i) {
        delete arr[i];
    }
    delete[] arr;

    auto t1 = clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
}

//замер UnqPtr
//массив не Tracked*, а UnqPtr<Tracked>
long long bench_unq(size_t N) {
    using clock = std::chrono::high_resolution_clock;

    Tracked::alive = 0;
    auto t0 = clock::now();

    UnqPtr<Tracked>* arr = new UnqPtr<Tracked>[N];
    for (size_t i = 0; i < N; ++i) {
        arr[i] = UnqPtr<Tracked>(new Tracked());
    }
    //массив разрушится сам, все объекты удалятся
    delete[] arr;

    auto t1 = clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
}

//замер ShrdPtr
//в ShrdPtr<Tracked>(new Tracked()) внутри выделяется счётчик 
long long bench_shrd(size_t N) {
    using clock = std::chrono::high_resolution_clock;

    Tracked::alive = 0;
    auto t0 = clock::now();

    ShrdPtr<Tracked>* arr = new ShrdPtr<Tracked>[N];
    for (size_t i = 0; i < N; ++i) {
        arr[i] = ShrdPtr<Tracked>(new Tracked());
    }
    delete[] arr;

    auto t1 = clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
}

//для STL shared_ptr
long long bench_stl(size_t N) {
    using clock = std::chrono::high_resolution_clock;

    Tracked::alive = 0;
    auto t0 = clock::now();

    std::shared_ptr<Tracked>* arr = new std::shared_ptr<Tracked>[N];
    for (size_t i = 0; i < N; ++i) {
        arr[i] = std::shared_ptr<Tracked>(new Tracked());
    }
    delete[] arr;

    auto t1 = clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
}

void print_header() {
    std::cout << std::left
        << std::setw(12) << "N"
        << std::setw(15) << "raw, ms"
        << std::setw(15) << "UnqPtr, ms"
        << std::setw(15) << "ShrdPtr, ms";
    std::cout << std::setw(15) << "STL shrd, ms";
    std::cout << "\n";

    std::cout << std::string(72, '-') << "\n";
}

void print_row(size_t N, long long raw, long long unq, long long shrd, long long stl) {
    std::cout << std::left
        << std::setw(12) << N
        << std::setw(15) << raw
        << std::setw(15) << unq
        << std::setw(15) << shrd;
    std::cout << std::setw(15) << stl;
    std::cout << "\n";
}

void bench_table() {
    const size_t sizes[] = {
        100,
        1000,
        10000,
        100000,
        1000000
    };
    const size_t count = sizeof(sizes) / sizeof(sizes[0]);

    std::cout << "\n=== Сравнение производительности умных указателей ===\n";
    std::cout << "sizeof(Tracked) = " << sizeof(Tracked) << " байт\n";
    std::cout << "sizeof(UnqPtr<Tracked>) = " << sizeof(UnqPtr<Tracked>) << " байт\n";
    std::cout << "sizeof(ShrdPtr<Tracked>) = " << sizeof(ShrdPtr<Tracked>) << " байт\n";
    std::cout << "sizeof(size_t) = " << sizeof(size_t) << " байт\n\n";

    print_header();

    for (size_t i = 0; i < count; ++i) {
        size_t N = sizes[i];

        long long raw = bench_raw(N);
        long long unq = bench_unq(N);
        long long shrd = bench_shrd(N);

        long long stl = bench_stl(N);
        print_row(N, raw, unq, shrd, stl);
    }
}

void print_memory_table() {
    const size_t sizes[] = { 100, 1000, 10000, 100000, 1000000 };
    const size_t count = sizeof(sizes) / sizeof(sizes[0]);

    std::cout << "\n=== Сравнение расхода памяти (теоретический, байт) ===\n";
    std::cout << std::left
        << std::setw(12) << "N"
        << std::setw(15) << "raw"
        << std::setw(15) << "UnqPtr"
        << std::setw(15) << "ShrdPtr"
        << "\n";
    std::cout << std::string(57, '-') << "\n";

    for (size_t i = 0; i < count; ++i) {
        size_t N = sizes[i];
        //(память под сами объекты Tracked) + (память под структуры, которые на них смотрят)
        size_t raw = N * sizeof(Tracked) + N * sizeof(Tracked*);
        size_t unq = N * sizeof(Tracked) + N * sizeof(UnqPtr<Tracked>);
        size_t shrd = N * sizeof(Tracked) + N * sizeof(ShrdPtr<Tracked>) + N * sizeof(size_t);

        std::cout << std::left
            << std::setw(12) << N
            << std::setw(15) << raw
            << std::setw(15) << unq
            << std::setw(15) << shrd
            << "\n";
    }
}