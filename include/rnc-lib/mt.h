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

#ifndef MT_H
#define MT_H

#include <rnc-lib/fq.h>
#include <stdint.h>

namespace rnc
{
namespace random
{
        typedef uint32_t random_type;

        typedef struct MT_STATE {
                random_type status[4];
                random_type mat1;
                random_type mat2;
                random_type tmat;
        } mt_state;

        void init(mt_state *state, random_type seed);

        inline void next_state(mt_state *state)
        {
                static const random_type no_sign = random_type(~0) >> 1;
                random_type x, y;

                y = state->status[3];
                x = (state->status[0] & no_sign)
                        ^ state->status[1]
                        ^ state->status[2];
                x ^= (x << 1);
                y ^= (y >> 1) ^ x;
                state->status[0] = state->status[1];
                state->status[1] = state->status[2];
                state->status[2] = x ^ (y << 10);
                state->status[3] = y;
                state->status[1] ^= -((int32_t)(y & 1)) & state->mat1;
                state->status[2] ^= -((int32_t)(y & 1)) & state->mat2;
        }

        inline random_type generate(mt_state *state)
        {
                next_state(state);

                random_type t0, t1;
                t0 = state->status[3];
                t1 = state->status[0] + (state->status[2] >> 8);
                t0 ^= t1;
                t0 ^= -((int32_t)(t1 & 1)) & state->tmat;

                return t0;
        }

        inline random_type generate_fq(mt_state *state)
        {
                return generate(state) % fq_size;
        }

        inline double generateP(mt_state *state)
        {
                static const double MAXVAL = double(random_type(~0));
                return generate(state)/MAXVAL;
        }
}
}

#endif //MT_H
