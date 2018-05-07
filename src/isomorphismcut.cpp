/**
 * isomorphismcut.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <string.h>
#include "common.h"
#include "isomorphismcut.h"

namespace vorpal::nibac {
    IsomorphismCut::IsomorphismCut(int maxindex, int pnumberIndices, int *pindices, double pviolation)
            : numberIndices(pnumberIndices), violation(pviolation), prev(0), next(0) {
        // Set up the array of indices.
        indices = new int[numberIndices];
        memcpy(indices, pindices, numberIndices * sizeof(int));

        // Set up a bitstring representing the indices.
        size = maxindex / sizeof(unsigned long) + (maxindex % sizeof(unsigned long) ? 1 : 0);
        bitstring = new unsigned long[size];
        memset(bitstring, 0, size * sizeof(unsigned long));
        for (int i = 0; i < numberIndices; ++i)
            bitstring[indices[i] / sizeof(unsigned long)] |= (1 << (indices[i] % sizeof(unsigned long)));
    }


    IsomorphismCut::~IsomorphismCut() {
        delete[] indices;
        delete[] bitstring;
    }


    IsomorphismCut *IsomorphismCut::setNext(IsomorphismCut *nextcut) {
        if (!nextcut) {
            next = 0;
            return this;
        }

        // We are just SETTING next, and not inserting, so we simply modify
        // the next pointer of this node and the prev pointer of the
        // next node. We do not modify the next.prev and nextcut.next.
        nextcut->prev = this;
        next = nextcut;

        return nextcut;
    }


    IsomorphismCut *IsomorphismCut::deleteCut(void) {
        IsomorphismCut *retval = next;
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = next;
        return retval;
    }


    bool IsomorphismCut::operator==(const IsomorphismCut &other) const {
        for (int i = 0; i < size; ++i)
            if (bitstring[i] != other.bitstring[i])
                return false;
        return true;
    }


    bool IsomorphismCut::operator<(const IsomorphismCut &other) const {
        unsigned long tester;
        for (int i = 0; i < size; ++i) {
            tester = other.bitstring[i] | bitstring[i];
            if (tester != other.bitstring[i])
                return false;
        }
        return true;
    }
};