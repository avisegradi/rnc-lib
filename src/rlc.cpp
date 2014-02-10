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

#include <rnc-lib/rlc.h>
#include <filemap>
#include <string.h>
#include <stdexcept>

// Autoindent hack
#define NS_BEGIN(a) namespace a {
#define NS_END(a)   }

NS_BEGIN(rnc)
NS_BEGIN(coding)

BlockList::BlockList(Block **blist, size_t count, size_t capacity, bool cleanup)
      : _cleanup(cleanup),
        _count(count),
        _capacity(capacity),
        _blocklist(blist)
{
}

BlockList::BlockList(size_t start_size, bool cleanup)
      : _cleanup(cleanup),
        _count(0),
        _capacity(start_size),
        _blocklist(static_cast<Block**>(malloc(_capacity*sizeof(Block*))))
{
}

BlockList::~BlockList()
{
        if (_cleanup)
                for (size_t i = 0; i<_count; i++)
                        delete _blocklist[i];
        free(_blocklist);
}

void BlockList::add(Block* blk)
{
        if (_count == _capacity)
        {
                _capacity <<= 1;
                _blocklist = static_cast<Block**>(
                        realloc(_blocklist, _capacity*sizeof(Block*)));
        }
        _blocklist[_count++] = blk;
}

void BlockList::drop(size_t index) throw (std::range_error)
{
        if (index >= _count) throw std::range_error("drop: index out of range");

        if (_cleanup)
                delete _blocklist[index];

        for (size_t to=index; to<_count-1; ++to)
                _blocklist[to] = _blocklist[to+1];

        --_count;
}

Matrix *BlockList::to_matrix(ToMatrixMode mode) const
{
        Row *rows = new Row[_count];

        size_t ncols = 0;
        Block **b = _blocklist;
        Row *r = rows;

        for (size_t i = 0; i<_count; ++i, ++b, ++r)
        {
                if (i == 0)
                {
                        ncols = mode==Coefficients
                                ? (*b)->coeff_count
                                : (*b)->block_length;
                }

                *r = mode==Coefficients
                        ? (*b)->coefficients
                        : (*b)->data;
        }

        return new Matrix(rows, _count, ncols);
}

void BlockList::to_matrices(Matrix **coefficients, Matrix **data) const
{
        Row *rows_c = new Row[_count];
        Row *rows_d = new Row[_count];

        size_t ncols_c = 0;
        size_t ncols_d = 0;
        Block **b = _blocklist;
        Row *rc = rows_c;
        Row *rd = rows_d;

        for (size_t i = 0; i<_count; ++i, ++b, ++rc, ++rd)
        {
                ncols_c = (*b)->coeff_count;
                ncols_d = (*b)->block_length;

                *rc = (*b)->coefficients;
                *rd = (*b)->data;
        }

        *coefficients = new Matrix(rows_c, _count, ncols_c);
        *data = new Matrix(rows_d, _count, ncols_d);
}

BlockList BlockList::random_sample(size_t size, random::mt_state *state) const
{
        if (size > _count)
                throw std::range_error("Sample size is too big");

        Block **sample = static_cast<Block**>(malloc(_count*sizeof(Block*)));

        memcpy(sample, _blocklist, _count*sizeof(Block*));
        random::shuffle(sample, _count, state);

        Block **result = static_cast<Block **>(realloc(sample, size*sizeof(Block*)));
        if (!result)
        {
                free(sample);
                throw std::runtime_error("realloc failed in random_sample()");
        }

        return BlockList(result, size, size, false);
}

void BlockList::random_drop(double p, size_t max_count, random::mt_state *state)
{
        size_t cnt = 0;
//        random::shuffle(*_blocklist, _count, state);
        for (size_t i=0; i<_count && cnt < max_count; ++i)
                if (random::generateP(state) < p)
                {
                        drop(i);
                        ++cnt;
                        --i;
                }
}

void BlockList::random_drop(random::mt_state *state)
{
        drop(random::generate(state) % _count);
}


BlockList File::block_list(Row coefficients[]) const
{
        BlockList bl(_nrows, true);

        for (size_t i = 0; i<_nrows; ++i)
        {
                Block *blk = new Block();
                blk->coefficients = coefficients[i];
                blk->data = _data + i*_ncols;
                blk->coeff_count  = _nrows;
                blk->block_length = _ncols;
                bl.add(blk);
        }

        return bl;
}

File::File(const std::string &path, const size_t nrows)
        : _data(0),
          _path(path),
          _padded(false),
          _nrows(nrows)
{
        FileMap_G<Element> fm(path);

        _file_size = fm.size();
        size_t _elem_count = _file_size / sizeof(Element);
        const size_t mod = _elem_count % _nrows;
        if (mod)
        {
                _elem_count += _nrows - mod;
                _padded = true;
        }
        _ncols = _elem_count / _nrows;

        _data_size = _elem_count * sizeof(Element);

        _data = reinterpret_cast<Element*>(malloc(_data_size));
        memcpy(_data, fm.addr(), _file_size);
        if (_padded)
                memset(_data + _file_size, 0, _data_size - _file_size);
}
File::~File()
{
        if (_data)
                free(_data);
}

void File::save_data(const std::string &path)
{
        FileMap_G<Element> outfile(path, O_SAVE, _file_size);
        memcpy(outfile.addr(), _data, _file_size);
}

NS_END(coding)
NS_END(rnc)
