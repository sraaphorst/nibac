/**
 * group.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef GROUP_H
#define GROUP_H

#include <set>
#include <string.h>
#include "common.h"
#include "permutationpool.h"

namespace vorpal::nibac {
    /**
     * Abstract superclass representing a permutation group.
     */
    class Group {
    protected:
        static int x;
        static int memsize;
        static PermutationPool *pool;
        static int *idperm;

    public:
        // Static initializers. These should automatically be called by subclass initialization
        // methods.
        static void initialize(int);

        static void destroy();

        Group() = default;
        virtual ~Group() = default;

        // Generic functions for manipulating permutations, which are
        // represented as arrays of int instead of as classes for the sake
        // of efficiency.
        // Multiply is viewed as applying the second perm, then the first.
        static inline void multiply(int *p1, int *p2, int *target) {
            static int i;
            for (i = 0; i < x; ++i)
                target[i] = p1[p2[i]];
        }


        static void invert(int *, int *);

        static inline bool isIdentity(int *p) {
            static int i;
            for (i = 0; i < x; ++i)
                if (p[i] != i)
                    return false;
            return true;
        }

        // Return the size of the base set.
        static inline int getBaseSetSize(void) { return x; }

        // Constructs the identity permutation.
        static inline void getIdentityPermutation(int *p) {
            memcpy(p, idperm, memsize);
        }

        // Get an arbitrary permutation from the group. This is required for Schreier-Sims algorithms.
        virtual int *getPermutation(int, int) = 0;

        virtual bool isCanonicalAndOrbInStab(int, int, std::set<int> &, int *,
                                             bool = true, bool = true, bool = true) = 0;

        // This is a method that is needed by Margot's algorithms in order to function
        // properly. It has a usage in the Schreier Sims representation of a group, but
        // not in any other, which is why we implement it here as an empty method.
        virtual void down(int, int);

        // A method used for bases: If we have a group G with a base imposed on the
        // elements, this returns the position of the element in the base. For instance,
        // in a Schreier-Sims scheme, we impose a base such that B[0] is the first element,
        // B[1] is the second, etc... so this method would return B^{-1}[i]. We implement
        // it here to just return the index of the variable as in most cases, unless there
        // is a base, this is what we want to do anyways.
        virtual int getPosition(int);

        // Get the element in the specified position of the base.
        virtual int getBaseElement(int);

#ifdef NODEGROUPS
        // Duplicates a group; used if we are storing a copy of a group at each node. The
        // copy must be manually deleted.
        virtual Group *makeCopy() = 0;
#endif
    };
};
#endif
