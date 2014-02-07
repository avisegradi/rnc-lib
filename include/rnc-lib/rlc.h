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
        public:
                BlockList(size_t start_size = 1, bool _cleanup = false);
                ~BlockList();

                void add(Block *blk);
                void drop(size_t index, bool cleanup = false) throw (std::range_error);
                inline size_t count() const { return _count; }
                inline size_t capacity() const { return _capacity; }
                inline Block** blocks() const { return _blocklist; }
        };

        class File
        {
                Element *_data;
                size_t _file_size;
                size_t _data_size;
                std::string _path;
                bool _padded;
                size_t _ncols;
        public:
                File(const std::string &path, const size_t ncols);
                ~File();

                inline Element *data() const { return _data; }
                inline operator Element*() const { return _data; }
                void save_data(const std::string &path);
                BlockList block_list() const;
        };

}
}

#endif //__RLC_H_
