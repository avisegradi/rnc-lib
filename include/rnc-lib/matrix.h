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

#ifndef MATRIX_H
#define MATRIX_H

#include <rnc-lib/fq.h>

namespace rnc
{
namespace matrix
{
        using namespace fq;

        extern int NCPUS;
        extern int BLK;

        void p(const fq_t v);
        void p(const fq_t *m, const int rows, const int cols);
        void p(const fq_t *m1, const fq_t *m2, const int rows, const int cols);

        void set_identity(fq_t *m, const int rows, const int cols) throw();
        void copy(const fq_t *m, fq_t *md, const int rows, const int cols) throw();
        bool invert(const fq_t *m_in, fq_t *res,
                    const int rows, const int cols) throw ();

        // (rows1 x cols1) * (cols1 x cols2) = (rows1 x cols2)
        void mul(const fq_t *m1, const fq_t *m2, fq_t *md,
                 const int rows1, const int cols1,int const cols2);
        void pmul(const fq_t *m1, const fq_t *m2, fq_t *md,
                 const int rows1, const int cols1,int const cols2);
        void rand_matr(fq_t *m, const int rows, const int cols);
}
}

#endif //MATRIX_H
