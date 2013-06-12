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

        bool equals(const fq_t * const m1, const fq_t * const m2) const {
                return 0 == memcmp(m1, m2, sizeof(fq_t) * _rows * _cols);
        }
};

class Bootstrap : public Matrix_TestCase
{
public:
        Bootstrap(size_t n, const int rows, const int cols)
                : Matrix_TestCase("Bootstrap (I==I)", n, rows, cols) {}

        bool performTest(ostream *buffer) const
        {
                fq_t *m1 = new fq_t [_rows * _cols];
                fq_t *m2 = new fq_t [_rows * _cols];

                set_identity(m1, _rows, _cols);
                set_identity(m2, _rows, _cols);

                if (buffer)
                {
                        (*buffer) << '(' << _rows << 'x' << _cols << ')';
                        // p(m1, m2, _rows, _cols, *buffer);
                }


                bool retval = equals(m1, m2);

                delete [] m1;
                delete [] m2;

                return retval;
        }
};

int main(int, char **)
{
        init_random();
        cout << "Seed=" << get_seed() << endl;

        init();

        cout << "Q=" << fq_size << endl;

        const int rowcounts[] = {1, 5, 10, 100, 0};
        const int colcounts[] = {1, 5, 10, 100, 0};
#define FORALL_ij                                       \
        for (int const * i = rowcounts; *i; i++)               \
                for (int const * j = colcounts; *j; j++)
#define FORALL_ij_square                        \
        FORALL_ij if (*i == *j)

        typedef list<TestCase*> case_list;
        case_list cases;
        FORALL_ij_square cases.push_back(new Bootstrap(5, *i, *j));

        int failed = 0;
        for (case_list::const_iterator i = cases.begin();
             i!=cases.end(); i++)
        {
                failed += (*i)->execute(cout);
        }

        return failed > 0;
}
