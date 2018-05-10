/**
 * superduper.c
 *
 * Code graciously provided by Dr. Rudi Mathon.
 * This is all based on some old C code and thus may look dated.
 */

#include <cassert>
#include "common.h"
#include "superduper.h"

namespace vorpal::nibac::design {
    // Static initializers.
    int SuperDuper::_C[100][100];
    int SuperDuper::flag_first_call = 0;

    SuperDuper::SuperDuper() = default;
    SuperDuper::~SuperDuper() = default;

    void SuperDuper::init_super_duper(int n) {
        assert(n >= 0 && n < 100);

        int i, j;
        for (i = 0; i <= n; i++) {
            _C[i][1] = i;
            _C[i][0] = 1;
        }
        for (j = 2; j <= n; j++) {
            _C[j][j] = 1;
            for (i = 0; i < j; i++) _C[i][j] = 0;
            for (i = j + 1; i <= n; i++)
                _C[i][j] = _C[i-1][j-1] + _C[i-1][j];
        }
        for (i = 0; i <= n; i++) {
            _C[i][1] = i;
            _C[i][0] = 1;
        }
        flag_first_call = n;
    }


    int SuperDuper::C(int n, int k) {
        assert(k >= 0 && n >= k);

        if (flag_first_call < n)
            init_super_duper(n);
        return _C[n][k];
    }


    int SuperDuper::super(int n, int k, int *vec) {
        int i, p;
        if (flag_first_call < n) init_super_duper(n);

        p = _C[n][k];
        for (i = 0; i < k; i++)
            p -= _C[n-vec[i]-1][k-i];
        return (p - 1);
    }


    void SuperDuper::duper(int n, int k, int r, int *vec) {
        int i, j, ni, ki, s;
        if (flag_first_call < n) init_super_duper(n);

        ni = _C[n][k];
        j = n;
        ki = k;
        s = r + 1;
        for (i = 0; i < k - 1; i++) {
            while (s > ni - _C[j][ki])
                j -= 1;
            vec[i] = n - j - 1;
            s += (_C[j+1][ki]-ni);
            ki -= 1;
            ni = _C[j][ki];
        }
        vec[k - 1] = n + s - ni - 1;
    }
};