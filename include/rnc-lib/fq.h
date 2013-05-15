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

namespace fq
{
#ifdef Q256
#pragma message ( "Using 8 bit finite field (Q256)" )
	typedef uint8_t fq_t;
#define fq_size 256
#define fq_groupsize 255
#else
#pragma message ( "Using 16 bit finite field (Q65536)" )
	typedef uint16_t fq_t;
#define fq_size 65536
#define fq_groupsize 65535
#endif //Q256

	extern const fq_t * const log_table;
	extern const fq_t pow_table[fq_size];

	void init();

	inline fq_t mul(fq_t a, fq_t b) {
		if (a&&b)
		{
			register int t = log_table[a] + log_table[b];
			if (t>fq_groupsize) t-=fq_groupsize;
			return pow_table[t];
		}
		else
			return 0;
	}
	inline fq_t inv(fq_t a) {
		return pow_table[fq_groupsize-log_table[a]]; }
	inline fq_t div(fq_t a, fq_t b) {
		if (a) {
			register int t = log_table[a] - log_table[b];
			if (t<0) t+=fq_groupsize;
			return pow_table[t];
		}
		else return 0;
	}
	inline fq_t add(fq_t a, fq_t b) {
		return a^b; }
	inline void mulby(fq_t& a, fq_t b) {
		a=mul(a, b);}
	inline void invert(fq_t& a) {
		a=inv(a); }
	inline void divby(fq_t& a, fq_t b) {		
		if (a) {
			int t = log_table[a] - log_table[b];
			if (t<0) t+=fq_groupsize;
			a = pow_table[t];
		}
	}
	inline void addto(fq_t& a, fq_t b) {
		a^=b; }
	inline void addto_mul(fq_t&d, fq_t a, fq_t b) {
		if (a&&b) {
			int t = log_table[a] + log_table[b];
			if (t>fq_groupsize) t-=fq_groupsize;
			d ^= pow_table[t];
		}
	}
}

#endif //FQ_H
