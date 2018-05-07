/**
 * bitstring.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "bitstring.h"

#define getbit(l, p)   ((l) &  (1 << (p)) ? 1 : 0)
#define setbit(l, p)   ((l) |= (1 << (p)))
#define clearbit(l, p) ((l) &= (ULONG_MAX ^ (1 << (p))))

namespace vorpal::nibac {
    bitstring::bitstring()
            : bits(nullptr), size(0), flipped(false) {
    }


    bitstring::~bitstring() {
        delete[] bits;
    }


    bitstring &bitstring::operator=(const bitstring &other) {
        if (bits)
            delete[] bits;
        bits = new unsigned long[other.size];
        size = other.size;
        flipped = other.flipped;
        memcpy(bits, other.bits, size * sizeof(unsigned long));
        return *this;
    }


    bitstring &bitstring::operator=(const unsigned long val) {
        if (bits)
            delete[] bits;
        bits = new unsigned long[1];
        size = 1;
        flipped = false;
        bits[0] = val;
        return *this;
    }


    bool bitstring::operator==(const bitstring &other) const {
        int max = (size > other.size ? size : other.size);

        for (int i = 0; i < max; ++i)
            if ((i < size ? bits[i] : (flipped ? ULONG_MAX : 0L))
                != (i < other.size ? other.bits[i] : (other.flipped ? ULONG_MAX : 0L)))
                return false;
        return true;
    }


    bool bitstring::operator==(const unsigned long val) const {
        if (size == 0 && val != 0L)
            return false;
        if (bits[0] != val)
            return false;

        for (int i = 1; i < size; ++i)
            if ((flipped && bits[i] != ULONG_MAX) || (!flipped && bits[i] != 0L))
                return false;
        return true;
    }


    bool bitstring::operator!=(const bitstring &other) const {
        return !(*this == other);
    }


    bool bitstring::operator!=(const unsigned long val) const {
        return !(*this == val);
    }


    int bitstring::operator[](int pos) const {
        int index = pos / (sizeof(unsigned long) * 8);
        return (index < size ? getbit(bits[index], pos % (sizeof(unsigned long) * 8)) : (flipped ? 1 : 0));
    }


    bool bitstring::operator<(const bitstring &other) const {
        int max = (size > other.size ? size : other.size);
        unsigned long val1, val2;

        for (int i = 0; i < max; ++i) {
            val1 = (i < size ? bits[i] : (flipped ? ULONG_MAX : 0L));
            val2 = (i < other.size ? other.bits[i] : (other.flipped ? ULONG_MAX : 0L));
            if (val1 < val2)
                return false;
            else if (val1 > val2)
                return true;
        }

        // They are equal
        return false;
    }


    void bitstring::set(int pos, int val) {
        int index = pos / (sizeof(unsigned long) * 8);
        int offset = pos % (sizeof(unsigned long) * 8);

        if (index >= size)
            resize(index + 1);

        if (val)
            setbit(bits[index], offset);
        else
            clearbit(bits[index], offset);
    }


    int bitstring::get(int pos) const {
        return (*this)[pos];
    }


    int bitstring::getSize() const {
        return size * sizeof(unsigned long) * 8;
    }


    void bitstring::flip() {
        flipped = (flipped ? false : true);
        unsigned long f = ULONG_MAX;

        // flip all the existing bits
        for (int i = 0; i < size; ++i)
            bits[i] ^= f;
    }


    void bitstring::resize(int newsize) {
        unsigned long *newbits = new unsigned long[newsize];
        memset(newbits, 0, newsize * sizeof(unsigned long));
        memcpy(newbits, bits, size * sizeof(unsigned long));

        if (newsize > size)
            for (int i = size; i < newsize; ++i)
                newbits[i] = (flipped ? ULONG_MAX : 0L);
        delete[] bits;
        bits = newbits;
        size = newsize;
    }


    void bitstring::band(const bitstring &other) {
        // Make sure that the sizes are the same
        if (other.size > size)
            resize(other.size);

        // Perform the and
        for (int i = 0; i < size; ++i) {
            bits[i] &= (i < other.size ? other.bits[i] : (other.flipped ? ULONG_MAX : 0L));
        }
    }


    void bitstring::bor(const bitstring &other) {
        // Make sure that the sizes are the same
        if (other.size > size)
            resize(other.size);

        // Perform the and
        for (int i = 0; i < size; ++i) {
            bits[i] |= (i < other.size ? other.bits[i] : (other.flipped ? ULONG_MAX : 0L));
        }
    }


    void bitstring::bxor(const bitstring &other) {
        // Make sure that the sizes are the same
        if (other.size > size)
            resize(other.size);

        // Perform the and
        for (int i = 0; i < size; ++i) {
            bits[i] ^= (i < other.size ? other.bits[i] : (other.flipped ? ULONG_MAX : 0L));
        }
    }


    int bitstring::getBitCount() const {
        int count = 0;
        int bound = sizeof(long) * 8;

        for (int i = 0; i < size; ++i)
            for (int j = 0; j < bound; ++j)
                if (bits[i] & (1 << j))
                    ++count;

        return count;
    }


#ifdef DEBUG
    std::ostream &operator<<(std::ostream &out, const bitstring &b)
    {
      int i;
      int bound = b.getSize();

      for (i=0; i < bound; ++i)
        out << b[i];
      out << " (";
      for (i=0; i < b.size; ++i)
        out << b.bits[i] << " ";
      out << ")";

      return out;
    }
#endif
};
