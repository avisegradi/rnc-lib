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
#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <mkstr>
#include <auto_arr_ptr>

using namespace std;
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
		string ex = MKStr() << "glib error: "
				    << context << ": " << error->message;
		g_error_free(error);
		throw ex;
	}
}

// Row address
#define RA(m,r)   ((m)+(r)*cols)
// Row element
#define RE(ra, c) (*(ra + (c)))
// Address
#define A(m,r,c) (RA(m,r)+(c))
// Element
#define E(m,r,c) (*A(m,r,c))

// Row address
#define RA_(m,r, cols)   ((m)+(r)*cols)
// Row element
#define RE_(ra, c, cols) RE(ra, c)
// Address
#define A_(m,r,c,cols) (RA_(m,r,cols)+(c))
// Element
#define E_(m,r,c,cols) (*A_(m,r,c,cols))

void p(const fq_t v)
{
        /// \todo setw(4) <- setw (ifdef(Q256) ? 2 : 4)
	cout << hex << setfill('0') << setw(4) << (int)v;
}

void p(const fq_t *m, const int rows, const int cols)
{
	for (int i=0; i<rows; ++i)
	{
		bool fcol = true;
		for (int j=0; j<cols; ++j)
		{
			if (fcol) fcol=false;
			else cout << ' ';

			p(E(m,i,j));
		}

		cout << endl;
	}
}
void p(const fq_t *m1, const fq_t *m2, const int rows, const int cols)
{
	for (int i=0; i<rows; ++i)
	{
		bool fcol = true;
		for (int j=0; j<cols; ++j)
		{
			if (fcol) fcol=false;
			else cout << ' ';

			p(E(m1,i,j));
		}
		cout << " | ";
		fcol = true;
		for (int j=0; j<cols; ++j)
		{
			if (fcol) fcol=false;
			else cout << ' ';

			p(E(m2,i,j));
		}

		cout << endl;
	}
}


void set_identity(fq_t *m, const int rows, const int cols) throw()
{
	fq_t* p = m;
	for (int i=0; i<rows; ++i)
		for (int j=0; j<cols; ++j, ++p)
			*p = (int)(i==j);
}

void copy(const fq_t *m, fq_t *md, const int rows, const int cols) throw()
{
	memcpy(md, m, rows*cols*sizeof(fq_t));
}

bool invert(const fq_t *m_in, fq_t *res, const int rows, const int cols) throw ()
{
	auto_arr_ptr<fq_t> m_ap = new fq_t [rows*cols];
	fq_t *m = m_ap;
	copy(m_in, m, rows, cols);
	set_identity(res, rows, cols);

	for (int i=0; i<rows; ++i)
	{
		fq_t *const m_i=RA(m,i), *const res_i=RA(res,i);

		//normalize row
		const fq_t p = RE(m_i,i);
		if (p == 0) return false;

		for (int c=0; c<cols; ++c)
		{
			if (c>=i) divby(RE(m_i,c), p);
			divby(RE(res_i,c), p);
		}

		for (int r=i+1; r<cols; ++r)
		{
			fq_t *const m_r=RA(m,r), *const res_r=RA(res,r);
			const fq_t h = RE(m_r,i);

			for (int c=0; c<cols; ++c)
			{
				if (c>=i) addto_mul(RE(m_r,c), RE(m_i,c), h);
				addto_mul(res_r[c], RE(res_i,c), h);
			}
		}
	}

	//back-substitution
	for (int i=rows-1; i>=0; --i)
	{
		fq_t *const res_i =RA(res,i);

		for (int r=i-1; r>=0; --r)
		{
			fq_t *const m_r = RA(m,r), *const res_r = RA(res,r);
			const fq_t h = RE(m_r,i);
			RE(m_r,i) = 0;

			for (int c=0; c<cols; ++c)
				addto_mul(RE(res_r,c), RE(res_i,c), h);
		}
	}

	return true;
}

typedef struct muldata
{
	const fq_t *m1;
	const fq_t *m2;
	fq_t *md;
	const int rows1;
	const int cols1;
	const int cols2;
} muldata;

void mulrow_blk(gpointer bb, gpointer d)
{
	const muldata *data = reinterpret_cast<muldata*>(d);

	const int b = BLOCK_SIZE;
	const int cols1 = data->cols1;
	const int cols2 = data->cols2;
	const int rows1 = data->rows1;
	fq_t * const md = data->md;
	const fq_t * const m1 = data->m1;
	const fq_t * const m2 = data->m2;

	int k, j;
	const long i = (long)bb - 1;
	const int li=i+b > rows1 ? rows1 : i+b;
	int lk, lj;
	int i0,j0,k0;

	for (j=0, lj=b; j < cols2; j+=b, lj+=b) {
		if (lj > cols2) lj=cols2;

		for (k=0, lk=b; k < cols1; k+=b, lk+=b) {
			if (lk > cols1) lk=cols1;

			for (i0=i; i0<li; ++i0) {
				for (k0=k; k0<lk; ++k0) {
					const fq_t e1 = E_(m1, i0, k0, cols1);

					for (j0=j; j0<lj; ++j0) {
						addto_mul(*A_(md, i0, j0, cols2), e1, E_(m2, k0, j0, cols2));
					}
				}
			}
		}
	}
}

void mulrow_nonblk(gpointer row, gpointer d)
{
	const long i = (long)row-1;
	const muldata *data = reinterpret_cast<muldata*>(d);
	const int cols2=data->cols2;
	const int cols1=data->cols1;
	fq_t * const md = data->md;
	const fq_t * const m1 = data->m1;
	const fq_t * const m2 = data->m2;

	int j, k;

	for (j=0; j<cols2; ++j)
	{
		fq_t s = 0;
		for (k=0; k<cols1; ++k) {
			s = add(s, fq::mul(E_(m1,i,k,cols1),
                                           E_(m2, k,j,cols2)));
		}
		E_(md,i,j,cols2) = s;
	}
}

void pmul_blk(const fq_t *m1, const fq_t *m2, fq_t *md,
	      const int rows1, const int cols1, int const cols2)
{
	struct muldata d = { m1, m2, md, rows1, cols1, cols2 };
	GError *error = 0;

	GThreadPool *pool = g_thread_pool_new(mulrow_blk, &d,
					      NCPUS, true, &error);
	checkGError("g_thread_pool_create", error);

	for (long i=1; i<=rows1; i+=BLOCK_SIZE) {
		g_thread_pool_push(pool, (void*)i, &error);
		checkGError("g_thread_pool_push", error);
	}

	g_thread_pool_free(pool, false, true);
}

void pmul_nonblk(const fq_t *m1, const fq_t *m2, fq_t *md,
	      const int rows1, const int cols1, int const cols2)
{
	struct muldata d = { m1, m2, md, rows1, cols1, cols2 };
	GError *error = 0;

	GThreadPool *pool = g_thread_pool_new(mulrow_nonblk, &d,
					      NCPUS, true, &error);
	checkGError("g_thread_pool_create", error);

	for (long i=1; i<=rows1; ++i) {
		g_thread_pool_push(pool, (void*)i, &error);
		checkGError("g_thread_pool_push", error);
	}

	g_thread_pool_free(pool, false, true);
}

void pmul(const fq_t *m1, const fq_t *m2, fq_t *md,
	 const int rows1, const int cols1, int const cols2)
{
	if (NCPUS == 1)
	{
		mul(m1, m2, md, rows1, cols1, cols2);
	}
	else
	{
		if (BLOCK_SIZE == 1)
			pmul_nonblk(m1, m2, md, rows1, cols1, cols2);
		else
			pmul_blk(m1, m2, md, rows1, cols1, cols2);
	}
}

void mul_nonblk(const fq_t *m1, const fq_t *m2, fq_t *md,
	 const int rows1, const int cols1,int const cols2)
{
	for (int i=0; i<rows1; ++i)
		for (int j=0; j<cols2; ++j)
		{
			fq_t s = 0;
			for (int k=0; k<cols1; ++k)
				addto_mul(s, E_(m1,i,k,cols1), E_(m2,k,j,cols2));
			E_(md,i,j,cols2) = s;
		}

}
void mul_blk(const fq_t *m1, const fq_t *m2, fq_t *md,
	 const int rows1, const int cols1,int const cols2)
{
	fq_t* delem = md;
	int i, j, k, i0,j0,k0, li, lj, lk;

	memset(md, 0, rows1*cols2*sizeof(fq_t));

	for (i=0, li=BLOCK_SIZE; i<rows1; li+=BLOCK_SIZE, i+=BLOCK_SIZE) {
		if (li > rows1) li=rows1;
		for (k=0, lk=BLOCK_SIZE; k<cols1; lk+=BLOCK_SIZE, k+=BLOCK_SIZE) {
			if (lk > cols1) lk=cols1;

			for (j=0, lj=BLOCK_SIZE; j<cols2; lj+=BLOCK_SIZE, j+=BLOCK_SIZE) {
				if (lj > cols2) lj=cols2;

				for (i0=i; i0<li; ++i0) {
					for (k0=k; k0<lk; ++k0) {
						const fq_t e1 =  E_(m1, i0, k0, cols1);
						for (j0=j; j0<lj; ++j0) {
							addto_mul(*A_(md, i0, j0, cols2), e1, E_(m2, k0, j0, cols2));
						}
					}
				}
			}
		}
	}

}

// (rows1 x cols1) * (cols1 x cols2) = (rows1 x cols2)
void mul(const fq_t *m1, const fq_t *m2, fq_t *md,
	 const int rows1, const int cols1,int const cols2)
{
	if (BLOCK_SIZE == 1)
		mul_nonblk(m1, m2, md, rows1, cols1, cols2);
	else
		mul_blk(m1, m2, md, rows1, cols1, cols2);
}

void rand_matr(fq_t *m, const int rows, const int cols)
{
	static int seed=0;
	if (!seed) {
		seed=1;
		srand(time(NULL));
	}

	for (int i=0;i<rows;++i)
		for (int j=0; j<cols; ++j)
			E(m,i,j) = rand() % fq_size;
}

}
}
