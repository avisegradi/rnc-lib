/* -*- mode: c++; coding: utf-8-unix -*-
 *
 * Copyright 2013 MTA SZTAKI
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

/**
   \file
   \brief Test matrix operations.
*/

#include <test.h>
#include <rnc>
#include <iostream>
#include <list>
#include <stdlib.h>
#include <string.h>

using namespace std;
using namespace rnc::test;
using namespace rnc::fq;
using namespace rnc::matrix;

rnc::random::mt_state rnd_state;

class Matrix_TestCase : public TestCase
{
protected:
        size_t _rows, _cols;
public:
        Matrix_TestCase(const string &tname, size_t n,
                        const int rows,
                        const int cols)
                : TestCase(string("matrix::") + tname, n),
                  _rows(rows),
                  _cols(cols)
        {}

        bool equals(Matrix &m1, Matrix &m2) const {
                if (m1.nrows != m2.nrows || m1.ncols != m2.ncols)
                        return false;

                const size_t rowsize = sizeof(Element) * _cols;
                size_t i;
                Row *r1 = m1.rows, *r2 = m2.rows;
                for (i=0; i<_rows; ++i, ++r1, ++r2)
                        if (0 != memcmp(*r1, *r2, rowsize)) return false;
                return true;
        }
};

class Bootstrap : public Matrix_TestCase
{
public:
        Bootstrap(size_t n, const int rows, const int cols)
                : Matrix_TestCase("Bootstrap (I==I)", n, rows, cols) {}

        bool performTest(ostream *buffer) const
        {
                Matrix m1(_rows, _cols);
                Matrix m2(_rows, _cols);

                set_identity(m1);
                set_identity(m2);

                if (buffer)
                {
                        (*buffer) << '(' << _rows << 'x' << _cols << ')';
                        // p(m1, m2, _rows, _cols, *buffer);
                }

                bool retval = equals(m1, m2);

                return retval;
        }
};

class RndEq : public Matrix_TestCase
{
public:
        RndEq(size_t n, const int rows, const int cols)
                : Matrix_TestCase("RndEq (A != B)", n, rows, cols) {}

        bool performTest(ostream *buffer) const
        {
                Matrix _A(_rows, _cols);
                Matrix _B(_rows, _cols);

                rand_matr(_A, &rnd_state);
                rand_matr(_B, &rnd_state);

                if (buffer)
                {
                        (*buffer) << '(' << _rows << 'x' << _cols << ')';
                        // p(m1, m2, _rows, _cols, *buffer);
                }

                bool retval = !equals(_A, _B);

                if (!retval && _rows == 5 && _cols==5)
                {
                        (*buffer) << '\n';
                        p(_A, _B, *buffer);
                }

                return retval;
        }
};

class Identity : public Matrix_TestCase
{
public:
        Identity(size_t n, const int rows, const int cols)
                : Matrix_TestCase("Identity (I*A=A)", n, rows, cols) {}

        bool performTest(ostream *buffer) const
        {
                Matrix _I(_rows, _rows);
                Matrix _A(_rows, _cols);
                Matrix _D(_rows, _cols);

                set_identity(_I);
                rand_matr(_A, &rnd_state);
                mul(_I, _A, _D);

                if (buffer)
                {
                        (*buffer) << '(' << _rows << 'x' << _cols << ')';
                        // p(m1, m2, _rows, _cols, *buffer);
                }

                bool retval = equals(_A, _D);

                return retval;
        }
};

class Inversion : public Matrix_TestCase
{
public:
        Inversion(size_t n, const int rows, const int cols)
                : Matrix_TestCase("Inversion (A * ~A = I)", n, rows, cols) {}

        bool performTest(ostream *buffer) const
        {
                Matrix _I(_rows, _cols);
                Matrix _A(_rows, _cols);
                Matrix _Ai(_rows, _cols);

                rand_matr(_A, &rnd_state);
                invert(_A, _Ai);

                pmul(_A, _Ai, _I);
                set_identity(_A);

                if (buffer)
                {
                        (*buffer) << '(' << _rows << 'x' << _cols << ')';
                        // p(m1, m2, _rows, _cols, *buffer);
                }

                bool retval = equals(_A, _I);

                return retval;
        }
};

class SparseMatrix : public Matrix_TestCase
{
        double _p;
public:
        SparseMatrix(size_t n, const int rows, const int cols, const double p)
                : Matrix_TestCase("Sparse matrix (cnt(0) ~= p)", n, rows, cols), _p(p) {}

        bool performTest(ostream *buffer) const
        {
                Matrix _P(_rows, _cols);

                rand_matr(_P, _p, &rnd_state);

                double sum = 0;
                Row *row = _P.rows;
                for (size_t i=0; i<_rows; ++i, ++row)
                {
                        unsigned int cnt0 = 0;
                        Row const p_i = *row;
                        for (size_t j=0; j<_cols; ++j)
                        {
                                if (!RE(p_i, j))
                                        ++cnt0;
                        }
                        sum += (double(cnt0)/_cols);
                }

                if (buffer)
                {
                        (*buffer) << '(' << _rows << 'x' << _cols << "); p = " << _p << "; avg(#0) = " << 1-sum/_rows << "; DIFF = " << (_p - 1 + sum/_rows);
                }


                bool retval = true;

                return retval;
        }
};


int main(int, char **)
{
        BLOCK_SIZE = 4;
        NCPUS = 2;

        init();
        rnc::random::random_type seed = time(NULL);
        cout << "Seed=" << seed << endl;
        rnc::random::init(&rnd_state, seed);

        cout << "Q=" << fq_size << endl;

        const int rowcounts[] = {1, 5, 10, 50, 100, 0};
        const int colcounts[] = {1, 5, 10, 50, 100, 0};
        const double ps[] = {0, 0.1, 1.0/3, 0.5, 0.75, 1, -1};
#define FORALL_ij                                       \
        for (int const * i = rowcounts; *i; ++i)               \
                for (int const * j = colcounts; *j; ++j)
#define FORALL_ij_square                        \
        FORALL_ij if (*i == *j)

        typedef list<TestCase*> case_list;
        case_list cases;
        FORALL_ij_square cases.push_back(new Bootstrap(1, *i, *j));
        FORALL_ij cases.push_back(new Identity(5, *i, *j));
        FORALL_ij if (*i>1 && *j>1) cases.push_back(new RndEq(5, *i, *j));
        FORALL_ij_square cases.push_back(new Inversion(5, *i, *j));
        FORALL_ij {
                for (const double *P = ps; *P >= 0; ++P)
                        cases.push_back(new SparseMatrix(5, *i, *j, *P));
        }

        Matrix ii(5, 5);
        set_identity(ii);
        p(ii, cout);
        rand_matr(ii, &rnd_state);
        p(ii, cout);

        int failed = 0;
        for (case_list::const_iterator i = cases.begin();
             i!=cases.end(); ++i)
        {
                failed += (*i)->execute(cout);
        }

        return failed > 0;
}
