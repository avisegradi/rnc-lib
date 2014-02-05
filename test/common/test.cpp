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

using namespace rnc::matrix;

fq::fq_t read(std::istream &)
{
        return 0;
}

void p(const Element v, ostream& buffer)
{
        /// \todo setw(4) <- setw (ifdef(Q256) ? 2 : 4)
        buffer << hex << setfill('0') << setw(4) << (int)v;
}

void p(const Matrix &m, ostream& buffer)
{
        CACHE_DIMS(m);

        Row *row = m.rows;
        for (int i=nrows; i>0; --i, ++row)
        {
                bool fcol = true;
                Element *elem = *row;
                for (size_t j=0; j<ncols; ++j, ++elem)
                {
                        if (fcol) fcol=false;
                        else buffer << ' ';
                        p(*elem, buffer);
                }

                buffer << endl;
        }
}
void p(const Matrix &m1, const Matrix &m2, ostream& buffer)
{
        CACHE_DIMS(m1);

        Row *rowA = m1.rows, *rowB = m2.rows;
        for (size_t i=0; i<nrows; ++i, ++rowA, ++rowB)
        {
                bool fcol = true;
                Element *elem = *rowA;
                for (size_t j=0; j<ncols; ++j, ++elem)
                {
                        if (fcol) fcol=false;
                        else buffer << ' ';

                        p(*elem, buffer);
                }

                buffer << " | ";
                fcol = true;
                elem = *rowB;
                for (size_t j=0; j<ncols; ++j, ++elem)
                {
                        if (fcol) fcol=false;
                        else buffer << ' ';

                        p(*elem, buffer);
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
