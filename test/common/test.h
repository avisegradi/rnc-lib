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
   \brief Utility functions for unit testing
*/

#ifndef TEST_H
#define TEST_H

#include <rnc>
#include <istream>

namespace rnc
{
namespace test
{
        using fq::fq_t;

        /// \addtogroup test Unit test utility functions
        /// @{
        /// \addtogroup output Output functions
        /// @{

        /// \brief Print a finite field element
        void p(const fq_t v);
        /// \brief Print a matrix of size \c rows x \c cols
        void p(const fq_t *m, const int rows, const int cols);
        /// \brief Print two matrices of the same size side-by-side
        void p(const fq_t *m1, const fq_t *m2, const int rows, const int cols);

        /// @}

        /// \addtogroup input Input functions
        /// @{

        /// \brief Read a single finite field element from the given input stream
        fq::fq_t read(std::istream &is);

        /// @}
        /// @}
}
}

#endif //TEST_H
