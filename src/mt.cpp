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

    \brief Mersenne Twister implementation
 */

#include <rnc-lib/mt.h>
#include <string.h>

namespace rnc
{
namespace random
{
        void init(mt_state *state, random_type seed)
        {
                random_type * const status = state->status;
                status[0] = seed;
                status[1] = state->mat1;
                status[2] = state->mat2;
                status[3] = state->tmat;
                for (int i = 1; i < 8; ++i) {
                        status[i & 3] ^= i + random_type(1812433253)
                                * (status[(i - 1) & 3]
                                   ^ (status[(i - 1) & 3] >> 30));
                }
                if (!((status[0] && (random_type(~0)>>1))
                      || status[1] || status[2] || status[3]))
                {
                        status[0] = 'A';
                        status[1] = 'L';
                        status[2] = 'M';
                        status[3] = 'A';
                }

                for (int i = 0; i < 8; ++i) {
                        next_state(state);
                }
        }
}
}
