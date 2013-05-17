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
   \brief Implementation of utility functions specified in test.h
*/

#include "test.h"

#include <iostream>
#include <iomanip>

using namespace std;

namespace rnc
{
namespace test
{

fq::fq_t read(std::istream &is)
{
        return 0;
}

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

}
}
