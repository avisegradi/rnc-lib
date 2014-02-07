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
                size_t _count;
                size_t _capacity;
                Block **_blocklist;
        public:
                BlockList(size_t start_size = 1);

                void add(Block *blk);
        };

        Element *load_file(const std::string &path);


}
}

#endif //__RLC_H_
