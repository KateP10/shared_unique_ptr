template <typename T>
class ShrdPtr {
private:
    T* ptr_;
    size_t* refCount_; //общий счетчик

    //friend class Ч объ€вл€ем все инстанциации ShrdPtr<U> друзь€ми
    template <typename U> friend class ShrdPtr; 

    void release() { //уменьшить счетчик и, если достигли нул€, удалить все
        if (refCount_ != nullptr) {
            --(*refCount_);
            if (*refCount_ == 0) {
                delete ptr_;
                delete refCount_;
            }
        }
    }

public:
    ShrdPtr() : ptr_(nullptr), refCount_(nullptr) {} //пустой

    explicit ShrdPtr(T* p) : ptr_(p), refCount_(nullptr) {
        if (p != nullptr) { //чтоб счетчик не висел на пустоту
            refCount_ = new size_t(1);
        }
    }

    //копирование
    ShrdPtr(const ShrdPtr& other) : ptr_(other.ptr_), refCount_(other.refCount_) {
        if (refCount_ != nullptr) ++(*refCount_);
    }
    ShrdPtr& operator=(const ShrdPtr& other) {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            refCount_ = other.refCount_;
            if (refCount_ != nullptr) ++(*refCount_);
        }
        return *this;
    }

    //перемещение
    ShrdPtr(ShrdPtr&& other) : ptr_(other.ptr_), refCount_(other.refCount_) {
        other.ptr_ = nullptr;
        other.refCount_ = nullptr;
    }
    ShrdPtr& operator=(ShrdPtr&& other) {
        if (this != &other) {
            //this->~ShrdPtr
            ptr_ = other.ptr_;
            release();
            other.ptr_ = nullptr;
            other.refCount_ = nullptr;
        }
        return *this;
    }

    //общий счетчик ссылок сохран€етс€ (указывает на один и тот же объект)
    //работает потому, что friend дает доступ к other.ptr_ и other.refCount_, а не€вное U* -> T* обеспечивает компил€тор
    template <typename U>
    ShrdPtr(const ShrdPtr<U>& other) : ptr_(other.ptr_), refCount_(other.refCount_) { //!!!!
        if (refCount_ != nullptr) ++(*refCount_);
    }

    ~ShrdPtr() {
        release();
    } 

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    T* get() const { return ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; }
    size_t use_count() const { return refCount_ ? *refCount_ : 0; } //сколько сейчас владельцев

    void reset(T* p = nullptr) { //заменить владение
        if (ptr_ != p) { 
            release();
            ptr_ = p;
            refCount_ = nullptr;
            if (p != nullptr) refCount_ = new size_t(1);
        }
    }
};