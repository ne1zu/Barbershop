#pragma once
#include <string>
#include "barbershop_exception.h"

class IndexOutOfRangeException : public BarbershopException {
private:
    int index;
    int size;

    static std::string makeMessage(int indexValue, int sizeValue) {
        std::string text = "Index " + std::to_string(indexValue) + " is out of range";
        if (sizeValue <= 0) return text + ", the collection is empty.";
        if (sizeValue == 1) return text + ", the only valid index is 0.";
        return text + ", valid indexes are 0.." + std::to_string(sizeValue - 1) + ".";
    }

public:
    IndexOutOfRangeException(int indexValue, int sizeValue)
        : BarbershopException(makeMessage(indexValue, sizeValue)), index(indexValue), size(sizeValue) {}

    int getIndex() const { return index; }
    int getSize() const { return size; }
};
