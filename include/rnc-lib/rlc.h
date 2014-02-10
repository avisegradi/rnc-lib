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

        struct Block
        {
                Row coefficients;
                Row data;
                size_t coeff_count;
                size_t block_length;
        };

        class BlockList
        {
                bool _cleanup;
                size_t _count;
                size_t _capacity;
                Block **_blocklist;
                BlockList(Block **blist, size_t count, size_t capacity, bool cleanup);
        public:
                BlockList(size_t start_size = 1, bool cleanup = false);
                ~BlockList();

                void add(Block *blk);
                void drop(size_t index) throw (std::range_error);
                inline size_t count() const { return _count; }
                inline size_t capacity() const { return _capacity; }
                inline Block** blocks() const { return _blocklist; }

                enum ToMatrixMode {
                        Coefficients,
                        Data
                };

                Matrix *to_matrix(ToMatrixMode mode) const;
                void to_matrices(Matrix **coefficients, Matrix **data) const;
                BlockList random_sample(size_t size, random::mt_state *state) const;
                void random_drop(double p, size_t max_count, random::mt_state *state);
                void random_drop(random::mt_state *state);
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
