#include "shared_ptr.h"
#include "unique_ptr.h"
#include <iostream>

//контейнер, показывающий, как использовать ShrdPtr<T> в реальном коде
template <typename T>
class SharedArray {
private:
    ShrdPtr<T>* data_; //динамический массив умных указателей
    size_t size_;
    size_t capacity_;

    void grow() { //удвоение capacity, копирование элементов, освобождение старого
        size_t newCap = (capacity_ == 0) ? 1 : capacity_ * 2;
        ShrdPtr<T>* newData = new ShrdPtr<T>[newCap];
        for (size_t i = 0; i < size_; ++i) {
            newData[i] = data_[i]; //копирующее присваивание ShrdPtr<T>, увеличивает счетчики
        }
        delete[] data_; //вызывает деструкторы всех ShrdPtr в старом массиве
        data_ = newData;
        capacity_ = newCap;
    }

public:
    SharedArray() : data_(nullptr), size_(0), capacity_(0) {}

    ~SharedArray() {
        delete[] data_; //для каждого элемента вызовется ~ShrdPtr<T>() -> release()
    }

    void push_back(const ShrdPtr<T>& item) { //принимаем по const-ссылке, чтобы не копировать лишний раз
        if (size_ == capacity_) grow();
        data_[size_++] = item;
    }

    ShrdPtr<T> get(size_t index) const {
        if (index >= size_) throw std::out_of_range("SharedArray::get");
        return data_[index];  // копия — счетчик увеличится
    }

    size_t size() const { return size_; }
};