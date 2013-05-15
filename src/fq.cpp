// -*- mode: c++; coding: utf-8-unix -*-

#include <rnc-lib/fq.h>
#include <mkstr>
#include <iostream>
#include <string>

using std::hex;
using std::string;

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
