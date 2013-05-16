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

/** \file fq.h
    \brief Finite field and operations
 */

#ifndef FQ_H
#define FQ_H

#include <config.h>
#include <stdint.h>

/// Random Network Coding Library namespace
namespace rnc
{

/**
   \brief Finite field definition and oprations.

   The finite field is represented using either 8 or 16 bit symbols. Operations
   are implemented with offline generated power- and discrete logarithm tables.

   The size of the field can only be 2^8 or 2^16 (8 bit and 16 bit finite
   fields). The 16 bit finite field is the default, as it nearly doubles the
   performance over fixed length blocks, and because generating a singular
   matrix in a 16 bit finite field is much less likely than in an 8 bit
   field. The size can be set to 8 bits by defining #Q256 to 1.
 */
namespace fq
{

/**
   \def Q256
   \brief Sets field size to 8 bits. (Default: 16 bits.)

   If set to 1, the size of the finite field will be 8 bits. By default, it is
   set to 0, in which case the field will be 16 bits. To set Q256 to 1, specify
   \e --with-q256 when running ./configure
 */

#if Q256 != 0
#pragma message ( "Using 8 bit finite field (Q256)" )
        typedef uint8_t fq_t; ///< Finite field type
#define fq_size 256
#define fq_groupsize 255
#else
#pragma message ( "Using 16 bit finite field (Q65536)" )
        typedef uint16_t fq_t;
#define fq_size 65536
#define fq_groupsize 65535
#endif //Q256

        /// \brief Discrete logarithm table
        extern const fq_t * const log_table;
        /// \brief Power table
        extern const fq_t pow_table[fq_size];

        /**
           \brief Initialize power- and logtables.

           \remark \e Must be called before performing any operations.
           \
         */
        void init();
-
        /// \addtogroup fqops Operations over the finite field
        /// @{

        /** \brief Multiplication over \f$\mathbb{F}_q\f$
            \return \f$ a*b\f$

            \test a == mul(1, a)
            \test mul(a, b) = mul(b, a) | a,b != 0
            \test mul(a, mul(b, c)) == mul(mul(a, b), c)
            \test a == mul(a, mul(a, inv(a))) | a != 0
         */
        inline fq_t mul(fq_t a, fq_t b) {
                if (a&&b)
                {
                        register int t = log_table[a] + log_table[b];
                        // === t%fq_groupsize; but this is faster.
                        if (t>fq_groupsize) t-=fq_groupsize;
                        return pow_table[t];
                }
                else
                        return 0;
        }
        /** \brief Multiplicative inverse over \f$\mathbb{F}_q\f$

         \return \f$a^{-1}\f$
         \test 1 == inv(1)
         \test a == mul(a, mul(a, inv(a))) | a != 0
        */
        inline fq_t inv(fq_t a) {
                return pow_table[fq_groupsize-log_table[a]]; }

        /** \brief Division over \f$\mathbb{F}_q\f$

            \return \f$a/b\f$
            \test a=mul(b, div(a, b)) | b != 0
         */
        inline fq_t div(fq_t a, fq_t b) {
                if (a) {
                        register int t = log_table[a] - log_table[b];
                        if (t<0) t+=fq_groupsize;
                        return pow_table[t];
                }
                else return 0;
        }

        /** \brief Addition over \f$\mathbb{F}_q\f$

            \return \f$a+b\f$

            \remark Addition over \f$\mathbb{F}_q\f$, if \f$q = 2^u\f$, is a
            simple XOR. No test is needed.
         */
        inline fq_t add(fq_t a, fq_t b) {
                return a^b; }

        /** \brief In-place multiplication over \f$\mathbb{F}_q\f$

            \test t:=a; mulby(t, b); t == mul(a, b)
        */
        inline void mulby(fq_t& a, fq_t b) {
                a=mul(a, b);}

        /** \brief In-place multiplicative inversion (\f$a^{-1}\f$) over
            \f$\mathbb{F}_q\f$

            \test t:=a; invert(t); t == inv(a) | a != 0
         */
        inline void invert(fq_t& a) {
                a=inv(a); }

        /** \brief In-place division over \f$\mathbb{F}_q\f$

            \test t:=a; divby(t, b); t == div(a, b) | b != 0
         */
        inline void divby(fq_t& a, fq_t b) {
                if (a) {
                        int t = log_table[a] - log_table[b];
                        if (t<0) t+=fq_groupsize;
                        a = pow_table[t];
                }
        }
        /** \brief In-place addition over \f$\mathbb{F}_q\f$

            \test t:=a; addto(t, b); t == add(a, b)
         */
        inline void addto(fq_t& a, fq_t b) {
                a^=b; }
        /** \brief Derived operation over \f$\mathbb{F}_q\f$: \f$d:=d+(a*b)\f$.

            An operation used very often in matrix multiplication. The
            specialized implementation performs better than #addto(d,
            #mul (a,b)).

            \test t:=d; addto_mul(t, a, b); t == add(d, mul(a, b))
         */
        inline void addto_mul(fq_t&d, fq_t a, fq_t b) {
                if (a&&b) {
                        int t = log_table[a] + log_table[b];
                        if (t>fq_groupsize) t-=fq_groupsize;
                        d ^= pow_table[t];
                }
        }

        ///@}
}
}

#endif //FQ_H
