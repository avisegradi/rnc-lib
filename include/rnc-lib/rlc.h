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

    \brief Random linear coding API
 */

#ifndef __RLC_H_
#define __RLC_H_

#include <rnc-lib/fq.h>
#include <rnc-lib/matrix.h>
#include <string>
#include <stdexcept>

namespace rnc
{
namespace coding
{

        using namespace fq;
        using namespace matrix;

        /** \brief Represents a block of coded data
         */
        struct Block
        {
                /** \brief Coefficients used to generate this block of data

                    Contains \c coeff_count elements
                 */
                Row coefficients;
                /** \brief Points to a block of data

                    Contains \c block_length elements
                 */
                Row data;
                /// \brief Length of \c coefficients
                size_t coeff_count;
                /// \brief Length of \c data
                size_t block_length;
        };

        /** \brief Stores a set of blocks

            A \c BlockList object provides a "view" to coded
            data. Multiple BlockLists can view the same data; thus,
            multiple subsets can be used for operations at the same time
            without copying the data.

            A BlockList can store an arbitrary number of \c Block objects.
            It uses an exponentially sized array list to do that: whenever
            the capacity of the list is exceeded, the array is reallocated
            with its size doubled. This avoids costly reallocations and
            slow list traversals at the possible cost of heap capacity.

            \remark Because of the exponential resizing, it is important
            to specify a reasonable starting capacity.

            \remark In most cases, the number of blocks in a file (\c
            BlockList::coeff_count) may be a perfect choice for starting
            capacity.

            \remark The array list contains only pointers, so the actual
            heap overhead is pretty low.
         */
        class BlockList
        {
                bool _cleanup;
                size_t _count;
                size_t _capacity;
                Block **_blocklist;
                BlockList(Block **blist, size_t count, size_t capacity, bool cleanup);
        public:
                /**
                   \brief Creates a copy of the \c BlockList object that
                          view the same data

                   \remark I.e.: The data itself is not copied; modifying
                           the data through one of these objects will
                           affect all copies.
                */
                BlockList shallow_copy();
                /**
                   \brief Creates a \c BlockList object

                   @param start_size Initial array capacity
                   @param cleanup Upon destructing this \c BlockList
                          object, destroy the underlying data too

                   \remarks Choose the initial capacity wisely to avoid
                            high heap overhead and costly reallocations.

                   \remarks Setting <tt>cleanup = true</tt> will free the
                            heap area containing the underlying data. Use
                            this when the \c BlockList object's life
                            cycle coincides with that of the data.

                   \remarks It may be a good idea to have such a \c
                            BlockList object—one whose lifecycle coincides
                            with that of the blocks comprising a coded
                            file. That is, the \c BlockList can be used as
                            a memory manager for blocks.

                   \remarks Among \c BlockList objects viewing the same
                            data, at most a single one can have \c cleanup
                            set to \c true . Having more than one freeing
                            the same heap space will cause a segmentation
                            fault.
                 */
                BlockList(size_t start_size = 1, bool cleanup = false);
                /** \brief Cleans up the \c BlockList object. If \c
                 *         cleanup is \c true , it will destroy the
                 *         underlying data also.
                 */
                ~BlockList();

                /** \brief Adds a \c Block to this set
                 */
                void add(Block *blk);
                /** \brief Drops the \c index th block from the set.
                 */
                void drop(size_t index) throw (std::range_error);
                /** \brief Number of blocks in this set
                 */
                inline size_t count() const { return _count; }
                /** \brief Number of blocks that can be stored without
                 *         reallocating the underlying array. This value
                 *         will change upon addition if necessary.
                 */
                inline size_t capacity() const { return _capacity; }
                /** \brief Array of blocks in this set.

                    \return The pointer to the heap area containing the
                            list of blocks. The list is \e not \c null
                            terminated; it contains \c count() elements.
                 */
                inline Block** blocks() const { return _blocklist; }

                /// \brief Input values for \c to_matrix .
                enum ToMatrixMode {
                        /// \brief The matrix returned will contain the
                        ///        coefficients of the blocks in this set
                        Coefficients,
                        /// \brief The matrix returned will contain the
                        ///        data of the blocks in this set
                        Data
                };

                /** \brief Generates a \c Matrix object containing the
                 *         specified information
                 *
                 *  \remark The returned matrix is allocated on the heap
                 *          by this function; it must be <tt>delete</tt>d
                 *          by the client code.
                 */
                Matrix *to_matrix(ToMatrixMode mode) const;
                /** \brief Generates two \c Matrix objects; one containing
                 *        the coefficients, and one containing the data of
                 *        the blocks in this set.
                 *
                 *  @param coefficients Output parameter: a pointer to the
                 *                      generated coefficient matrix
                 *  @param data         Output parameter: a pointer to the
                 *                      generated data matrix
                 *
                 *  \remark The matrices returned will be allocated on
                 *          the heap by this function; they must be
                 *          <tt>delete</tt>d by the client code.
                 */
                void to_matrices(Matrix **coefficients, Matrix **data) const;
                BlockList random_sample(size_t size, random::mt_state *state) const;
                void random_drop(double p, size_t max_count, random::mt_state *state);
                Block *random_drop(random::mt_state *state);
                inline Block *random_block(random::mt_state *state)
                {
                        return _blocklist[random::generate(state) % _count];
                }
        };

        class File
        {
                Element *_data;
                size_t _file_size;
                size_t _data_size;
                std::string _path;
                bool _padded;
                size_t _nrows;
                size_t _ncols;
        public:
                File(const std::string &path, const size_t nrows);
                ~File();

                inline Element *data() const { return _data; }
                inline operator Element*() const { return _data; }
                void save_data(const std::string &path);
                BlockList block_list(Row coefficients[]) const;
                inline size_t ncols() const { return _ncols; }
                inline size_t nrows() const { return _nrows; }
        };

}
}

#endif //__RLC_H_
