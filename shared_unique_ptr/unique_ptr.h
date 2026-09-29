template <typename T>
class UnqPtr {
private:
    T* ptr_; //просто сырой указатель, тут счетчик не нужен

public:
    UnqPtr() : ptr_(nullptr) {} //пуста€ инициализаци€
    explicit UnqPtr(T* p) : ptr_(p) {} //explicit запрещает не€вные преобразовани€ вида UnqPtr<X> u = new X

    ~UnqPtr() { delete ptr_; }

    //копирование запрещено (€вно удал€ем копирующий конструктор и копирующий оператор присваивани€)
    //delete: если кто-то попробует скопировать - ошибка компил€ции
    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    //перемещение
    UnqPtr(UnqPtr&& other) noexcept : ptr_(other.ptr_) { //noexcept Ч это обещание компил€тору, что функци€ никогда не выбрасывает исключений
        //если это обещание нарушитс€ (внутри все-таки вылетит исключение), программа не продолжит работать нормально Ч она вызовет std::terminate() и аварийно завершитс€
        other.ptr_ = nullptr;
    }
    UnqPtr& operator=(UnqPtr&& other) {
        if (this != &other) { //иначе delete ptr_ уничтожил бы объект до того, как мы его скопировали
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    //работает, если U* конвертируетс€ в T*, если нет Ч ошибка компил€ции
    template <typename U>
    UnqPtr(UnqPtr<U>&& other) : ptr_(other.release()) {}

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    T* get() const { return ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; } //позвол€ет писать if (u) { ... }

    T* release() { //отдать сырой указатель, сн€в с себ€ владение, примен€етс€ при передаче объекта наружу
        T* tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }

    void reset(T* p = nullptr) { //заменить владение, сначала удал€ем старый объект, потом кладем новый
        if (ptr_ != p) {
            delete ptr_;
            ptr_ = p;
        }
    }
};