/**
 * isomorphismcut.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef ISOMORPHISMCUT_H
#define ISOMORPHISMCUT_H

#include "common.h"

namespace vorpal::nibac {
    /**
     * Encapsulates a doubly linked list of isomorphism cuts;
     * allows such operations as checking for equality, containment, etc...
     */
    class IsomorphismCut final {
    private:
        int numberIndices;
        int *indices;
        int size;
        unsigned long *bitstring;
        double violation;
        IsomorphismCut *prev, *next;

    public:
        IsomorphismCut(int, int, int *, double);

        virtual ~IsomorphismCut();

        IsomorphismCut *setNext(IsomorphismCut *);

        inline IsomorphismCut *getNext(void) { return next; }

        IsomorphismCut *deleteCut(void);

        inline int *getIndices(void) { return indices; }

        inline int getNumberIndices(void) { return numberIndices; }

        inline double getViolation(void) { return violation; }

        bool operator==(const IsomorphismCut &) const;

        // Containment (is this contained in other?)
        bool operator<(const IsomorphismCut &) const;
    };
};
#endif
