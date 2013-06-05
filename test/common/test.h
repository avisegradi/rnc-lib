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
#include <iostream>
#include <string>

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
        void p(const fq_t v, std::ostream& buffer = std::cout);
        /// \brief Print a matrix of size \c rows x \c cols
        void p(const fq_t *m, const int rows, const int cols,
               std::ostream& buffer = std::cout);
        /// \brief Print two matrices of the same size side-by-side
        void p(const fq_t *m1, const fq_t *m2, const int rows, const int cols,
               std::ostream& buffer = std::cout);

        /// @}

        /// \addtogroup input Input functions
        /// @{

        /// \brief Read a single finite field element from the given input stream
        fq::fq_t read(std::istream &is);

        /// @}
        /// @}

        /** \brief Represents a single test case

            Abstract class; template function pattern.

            Example:
            \code
class MyTestCase : public TestCase
{
        int _shouldBe;
public:
        MyTestCase(const std::string &__name, int __shouldBe)
                : TestCase(__name), _shouldBe(__shouldBe)
        {}

        bool performTest(MKStr *buffer)
        {
                if (1 == __shouldBe)
                {
                        if (buffer)
                                 (*buffer) << "1 should be " << __shouldBe;
                        return false;
                }
                else
                {
                        return true;
                }
        }
};

int main()
{
        MyTestCase tc("my_test_case(5)", 5);
        tc.execute(cout);
}
            \endcode
         */
        class TestCase
        {
                /// \brief Name of the test case
                const std::string _name;
                /// \brief Number of times to repeat when #execute is called
                const size_t _repeat;

        public:
                static std::string field_separator;

                /** \brief Constructor

                    \param __name   Name of the test case. This will be printed
                                    out by #execute
                    \param __repeat Number of times to repeat when #execute is
                                    called
                 */
                TestCase(const std::string &__name, size_t __repeat = 1);

                /** \brief Perform the test.

                    \retval true  The test has passed.
                    \retval false The test has failed. Additional detail may be ouput in
                                  buffer.

                    \param buffer [optional] Output buffer. If NULL, no output
                                  must be generated.
                 */
                virtual bool performTest(std::ostream *buffer = 0) const = 0;

                /** \brief The name of the test case */
                const std::string& name() const { return _name; }
                /** \brief Number of times to repeat when #execute is calledtes */
                const size_t repeat() const { return _repeat; }

                /** \brief Generate output based on the result of #performTest

                    Calls #performTest #repeat times and generates output based
                    on its result.

                    \param buffer Output buffer
                    \return Number of \e failed jobs

                    Example
                    \code
test_1_1	1/1	PASS
test_2_1	1/2	PASS
test_2_1	2/2	FAIL	Details provided by performTest
                    \endcode
                 */
                int execute(std::ostream& buffer) const;
        };
}
}

#endif //TEST_H
