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

/** \file

    \brief Implementation of the matrix operations specified in rnc-lib/matrix.h
 */

#include <rnc-lib/matrix.h>
#include <time.h>
#include <string.h>
#include <glib.h>
#include <mkstr>
#include <auto_arr_ptr>
#include <string>

using namespace rnc::fq;

namespace rnc
{
namespace matrix
{

int BLOCK_SIZE = 1;
int NCPUS = 2;

static void checkGError(char const * const context, GError *error)
{
        if (error != 0)
        {
                std::string ex = MKStr() << "glib error: "
                                    << context << ": " << error->message;
                g_error_free(error);
                throw ex;
        }
}

void set_identity(Matrix &m) throw()
{
        CACHE_DIMS(m);

        Row *row = m.rows;
        for (size_t i=0; i<nrows; ++i, ++row)
        {
                Element *elem = *row;
                for (size_t j=0; j<ncols; ++j, ++elem)
                        *elem = i==j ? 1 : 0;
        }
}

void set_zero(Matrix &m) throw()
{
        CACHE_DIMS(m);
        const size_t rowsize = ncols * sizeof(Element);

        Row *row = m.rows;
        for (size_t i=0; i<nrows; ++i, ++row)
        {
                memset(*row, 0, rowsize);
        }
}

void copy(const Matrix &m, Matrix &md) throw()
{
        CACHE_DIMS(m);
        const size_t rowsize = ncols*sizeof(Element);

        Row *row = m.rows, *rowD = md.rows;
        for (size_t i=0; i<nrows; ++i, ++row, ++rowD)
                memcpy(*rowD, *row, rowsize);
}

void copy(const Matrix &m, Element* dest) throw()
{
        CACHE_DIMS(m);
        const size_t rowsize = ncols*sizeof(Element);

        Row *row = m.rows;
        Element *d = dest;
        for (size_t i=0; i<nrows; ++i, ++row, d+=ncols)
                memcpy(d, *row, rowsize);
}

bool invert(const Matrix &m_in, Matrix &res) throw ()
{
        CACHE_DIMS(m_in);

        Matrix m(nrows, ncols);
        copy(m_in, m);
        set_identity(res);

        Row *rm = m.rows;
        Row *rd = res.rows;
        for (size_t i=0; i<nrows; ++i, ++rm, ++rd)
        {
                Row const m_i = *rm;
                Row const res_i = *rd;

                //normalize row
                const Element p = RE(m_i,i);
                if (p == 0) return false; // \todo: row-switch

                for (size_t c=0; c<ncols; ++c)
                {
                        if (c>=i) divby(RE(m_i,c), p);
                        divby(RE(res_i,c), p);
                }

                Row *frm = rm+1;
                Row *frd = rd+1;
                for (size_t r=i+1; r<ncols; ++r, ++frm, ++frd)
                {
                        Row const m_r = *frm;
                        Row const res_r = *frd;
                        const Element h = RE(m_r,i);

                        for (size_t c=0; c<ncols; ++c)
                        {
                                if (c>=i) addto_mul(RE(m_r,c), RE(m_i,c), h);
                                addto_mul(RE(res_r, c), RE(res_i,c), h);
                        }
                }
        }

        //back-substitution
        rd = res.rows + nrows - 1;
        rm = m.rows + nrows - 1;
        for (int i=nrows-1; i>=0; --i, --rd, --rm)
        {
                Row const res_i = *rd;

                Row *rbd = rd - 1;
                Row *rbm = rm - 1;
                for (int r = i - 1; r>=0; --r, --rbd, --rbm)
                {
                        Row const res_r = *rbd;
                        Row const m_r = *rbm;
                        const Element h = RE(m_r,i);
                        RE(m_r,i) = 0;

                        for (size_t c=0; c<ncols; ++c)
                                addto_mul(RE(res_r,c), RE(res_i,c), h);
                }
        }

        return true;
}

/** \brief Description of a single workunit

    Matrix multiplication threads process workunits described with this
    construct.
 */
typedef struct muldata
{
        /// \brief Left-hand side matrix
        const Matrix &m1;
        /// \brief Right-hand side matrix
        const Matrix &m2;
        /// \brief Result address
        Matrix &md;
} muldata;

void mulrow_blk(gpointer bb, gpointer d)
{
        muldata *data = reinterpret_cast<muldata*>(d);

        const int b = BLOCK_SIZE;
        const size_t cols1 = data->m1.ncols;
        const size_t cols2 = data->m2.ncols;
        const size_t rows1 = data->m1.nrows;
        Matrix &md = data->md;
        const Matrix &m1 = data->m1;
        const Matrix &m2 = data->m2;

        size_t k, j;
        const size_t i = (size_t)bb - 1;
        const size_t li=i+b > rows1 ? rows1 : i+b;
        size_t lk, lj;
        size_t i0,j0,k0;

        for (j=0, lj=b; j < cols2; j+=b, lj+=b) {
                if (lj > cols2) lj=cols2;

                for (k=0, lk=b; k < cols1; k+=b, lk+=b) {
                        if (lk > cols1) lk=cols1;

                        for (i0=i; i0<li; ++i0) {
                                for (k0=k; k0<lk; ++k0) {
                                        const Element e1 = E(m1, i0, k0);
                                        for (j0=j; j0<lj; ++j0) {
                                                addto_mul(E(md, i0, j0),
                                                          e1,
                                                          E(m2, k0, j0));
                                        }
                                }
                        }
                }
        }
}

void mulrow_nonblk(gpointer row, gpointer d)
{
        const size_t i = (size_t)row-1;
        muldata *data = reinterpret_cast<muldata*>(d);
        const size_t cols2=data->m2.ncols;
        const size_t cols1=data->m1.ncols;
        Matrix &md = data->md;
        const Matrix &m1 = data->m1;
        const Matrix &m2 = data->m2;

        size_t j, k;

        for (j=0; j<cols2; ++j)
        {
                Element s = 0;
                for (k=0; k<cols1; ++k) {
                        s = add(s, fq::mul(E(m1,i,k),
                                           E(m2,k,j)));
                }
                E(md,i,j) = s;
        }
}

void pmul_blk(const Matrix &m1, const Matrix &m2, Matrix &md)
{
        const size_t rows1 = m1.nrows;
        struct muldata d = { m1, m2, md };
        GError *error = 0;

        GThreadPool *pool = g_thread_pool_new(mulrow_blk, &d,
                                              NCPUS, true, &error);
        checkGError("g_thread_pool_create", error);

        for (size_t i=1; i<=rows1; i+=BLOCK_SIZE) {
                g_thread_pool_push(pool, (void*)i, &error);
                checkGError("g_thread_pool_push", error);
        }

        g_thread_pool_free(pool, false, true);
}

void pmul_nonblk(const Matrix &m1, const Matrix &m2, Matrix &md)
{
        const size_t rows1 = m1.nrows;
        struct muldata d = { m1, m2, md };
        GError *error = 0;

        GThreadPool *pool = g_thread_pool_new(mulrow_nonblk, &d,
                                              NCPUS, true, &error);
        checkGError("g_thread_pool_create", error);

        for (size_t i=1; i<=rows1; ++i) {
                g_thread_pool_push(pool, (void*)i, &error);
                checkGError("g_thread_pool_push", error);
        }

        g_thread_pool_free(pool, false, true);
}

void pmul(const Matrix &m1, const Matrix &m2, Matrix &md)
{
        if (NCPUS == 1)
        {
                mul(m1, m2, md);
        }
        else
        {
                if (BLOCK_SIZE == 1)
                        pmul_nonblk(m1, m2, md);
                else
                        pmul_blk(m1, m2, md);
        }
}


void mul_nonblk(const Matrix &m1, const Matrix &m2, Matrix &md)
{
        const size_t rows1 = m1.nrows;
        const size_t cols1 = m1.ncols;
        const size_t cols2 = m2.ncols;
        for (size_t i=0; i<rows1; ++i)
                for (size_t j=0; j<cols2; ++j)
                {
                        Element s = 0;
                        for (size_t k=0; k<cols1; ++k)
                                addto_mul(s, E(m1,i,k), E(m2,k,j));
                        E(md,i,j) = s;
                }

}

void mul_blk(const Matrix &m1, const Matrix &m2, Matrix &md)
{
        size_t i, j, k, i0,j0,k0, li, lj, lk;

        const size_t cols1 = m1.ncols;
        const size_t cols2 = m2.ncols;
        const size_t rows1 = m1.nrows;

        Row *r = md.rows;
        const size_t rowsize = cols2 * sizeof(Element);
        for (i=0; i<rows1; ++i, ++r)
                memset(*r, 0, rowsize);

        for (i=0, li=BLOCK_SIZE; i<rows1; li+=BLOCK_SIZE, i+=BLOCK_SIZE) {
                if (li > rows1) li=rows1;
                for (k=0, lk=BLOCK_SIZE; k<cols1; lk+=BLOCK_SIZE, k+=BLOCK_SIZE) {
                        if (lk > cols1) lk=cols1;

                        for (j=0, lj=BLOCK_SIZE; j<cols2; lj+=BLOCK_SIZE, j+=BLOCK_SIZE) {
                                if (lj > cols2) lj=cols2;

                                for (i0=i; i0<li; ++i0) {
                                        for (k0=k; k0<lk; ++k0) {
                                                const Element e1 =  E(m1, i0, k0);
                                                for (j0=j; j0<lj; ++j0) {
                                                        addto_mul(E(md, i0, j0),
                                                                  e1,
                                                                  E(m2, k0, j0));
                                                }
                                        }
                                }
                        }
                }
        }
}

// (rows1 x cols1) * (cols1 x cols2) = (rows1 x cols2)
void mul(const Matrix &m1, const Matrix &m2, Matrix &md)
{
        if (BLOCK_SIZE == 1)
                mul_nonblk(m1, m2, md);
        else
                mul_blk(m1, m2, md);
}

void rand_matr(Matrix &m, random::mt_state *rnd_state)
{
        CACHE_DIMS(m);

        Row *row = m.rows;
        for (size_t i=0; i<nrows; ++i, ++row)
        {
                Element *elem = *row;
                for (size_t j=0; j<ncols; ++j, ++elem)
                        *elem = random::generate_fq(rnd_state);
        }
}

}
}
