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

namespace rnc
{
namespace coding
{

BlockList::BlockList(size_t start_size)
        : _count(0),
          _capacity(start_size),
          _blocklist(static_cast<Block**>(malloc(_capacity*sizeof(Block*))))
{
}

void BlockList::add(Block* blk)
{
        if (_count == _capacity)
        {
                _capacity *= 2;
                _blocklist = static_cast<Block**>(
                        realloc(_blocklist, _capacity*sizeof(Block*)));
        }
        _blocklist[_count++] = blk;
}

BlockList File::block_list() const
{
        BlockList bl(_data_size / _ncols);

        return bl;
}

File::File(const std::string &path, const size_t ncols)
        : _data(0),
          _path(path),
          _padded(false),
          _ncols(ncols)
{
        FileMap_G<Element> fm(path);

        _file_size = fm.size();
        size_t _elem_count = _file_size / sizeof(Element);
        if (_elem_count * sizeof(Element) < _file_size)
        {
                _elem_count += ncols;
                _padded = true;
        }
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

}
}
