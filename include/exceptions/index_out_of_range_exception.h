#pragma once
#include <string>
#include "barbershop_exception.h"

class IndexOutOfRangeException : public BarbershopException {
private:
    int index;
    int size;

    static std::string makeMessage(int indexValue, int sizeValue) {
        std::string text = "Индекс " + std::to_string(indexValue) + " вне допустимого диапазона";
        if (sizeValue <= 0) return text + ", коллекция пуста.";
        if (sizeValue == 1) return text + ", допустим только индекс 0.";
        return text + ", допустимые индексы: 0.." + std::to_string(sizeValue - 1) + ".";
    }

public:
    IndexOutOfRangeException(int indexValue, int sizeValue)
        : BarbershopException(makeMessage(indexValue, sizeValue)), index(indexValue), size(sizeValue) {}

    int getIndex() const { return index; }
    int getSize() const { return size; }
};
