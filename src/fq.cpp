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

#include <rnc-lib/fq.h>
#include <mkstr>
#include <iostream>
#include <string>

using std::hex;
using std::string;

namespace rnc
{
namespace fq
{

static fq_t ltab[fq_size];

void init()
{
	for (int i=0; i<fq_size-1; i++)
		ltab[pow_table[i]] = i;

#ifdef VERIFY_FQ
	for (int i=0; i<fq_size;i++)
		for (int j=0;j<fq_size;j++)
		{
			if ((i==0 || j==0) && mul(i,j)!=0) {
				throw string(MKStr() << "Multiplication is "
					     << "inconsistent ("
					     << hex << i
					     << " * " << j << ")="
					     << (int)mul(i, j));
			}
			else if (i==1 && mul(i, j) != j) {
				throw string(MKStr() << "Multiplication is "
					     << "inconsistent ("
					     << hex << i
					     << " * " << j << ")="
					     << (int)mul(i, j));
			}
			else if (j!=0 && mul(j, div(i, j)) != i)
			{
				throw string(MKStr() << "Multiplication is "
					     << "inconsistent ("
					     << hex << j
					     << " * (" << i << "/ " << j << "))="
					     << (int)mul(j, div(i, j)));
			}

			if (mul(i,j) != mul(j,i))
				throw string(MKStr() << "Multiplication is not "
					     << "not commutative ("
					     << hex << i << ", " << j << ')');
		}
#endif
}

const fq_t pow_table[fq_size] =
#ifdef Q256
#include "pow_table_8"
#else
#include "pow_table_16"
#endif //Q256
;
const fq_t * const log_table = ltab;

}
}
