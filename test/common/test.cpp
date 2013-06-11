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
#include <sstream>

using namespace std;

namespace rnc
{
namespace test
{

using rnc::matrix::Matrix;

fq::fq_t read(std::istream &is)
{
        return 0;
}

void p(const fq_t v, ostream& buffer)
{
        /// \todo setw(4) <- setw (ifdef(Q256) ? 2 : 4)
	buffer << hex << setfill('0') << setw(4) << (int)v;
}

void p(const Matrix m, const int rows, const int cols, ostream& buffer)
{
	for (int i=0; i<rows; ++i)
	{
		bool fcol = true;
		for (int j=0; j<cols; ++j)
		{
			if (fcol) fcol=false;
			else buffer << ' ';

			p(E(m,i,j), buffer);
		}

		buffer << endl;
	}
}
void p(const Matrix m1, const Matrix m2,
       const int rows, const int cols, ostream& buffer)
{
	for (int i=0; i<rows; ++i)
	{
		bool fcol = true;
		for (int j=0; j<cols; ++j)
		{
			if (fcol) fcol=false;
			else buffer << ' ';

			p(E(m1,i,j), buffer);
		}
		buffer << " | ";
		fcol = true;
		for (int j=0; j<cols; ++j)
		{
			if (fcol) fcol=false;
			else buffer << ' ';

			p(E(m2,i,j), buffer);
		}

		buffer << endl;
	}
}

string TestCase::field_separator = "\t";

        TestCase::TestCase(const string &__name, size_t __repeat)
        : _name(__name), _repeat(__repeat)
{}

int TestCase::execute(std::ostream& buffer) const
{
        int failed = 0;
        for (size_t i=1; i<=repeat(); i++)
        {
                buffer << name() << field_separator
                       << i << '/' << repeat() << field_separator;
                ostringstream buf;
                if (performTest(&buf))
                        buffer << "PASS";
                else {
                        ++failed;
                        buffer << "FAIL";
                }
                buffer << field_separator << buf.str() << endl;
        }
        return failed;
}

}
}
