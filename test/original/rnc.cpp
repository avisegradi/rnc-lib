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

    \brief Original application, which has been turned into a library.

    \remark This code is retained for later use and testing.
 */

#include <glib.h>
#include <rnc>
#include <iostream>
#include <time.h>
#include <exception>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "mkstr"
#include "auto_arr_ptr"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;
using namespace rnc;
using namespace rnc::fq;
using namespace rnc::matrix;

template <class T>
class SetFinally
{
	T *_this;
public:
	SetFinally(T *_this) : _this(_this) {}
	~SetFinally() { if (_this) _this->release(); }
	void cancel() { _this = 0; }
};

template <class T>
class FileMap_G
{
	const string _path;
	int _fd;
	off_t _size;
	T *_addr;
public:
	static void save(T* ptr, off_t length, const string &path)
	{
		FileMap_G<T> fm = FileMap_G<T>(path, O_RDWR|O_CREAT|O_TRUNC, length);
		memcpy(fm.addr(), ptr, length);
	}

	FileMap_G(const string &path, int flags = O_RDONLY, off_t length = 0)
		: _path(path), _fd(0),
		  _size(length),
		  _addr(0)
	{
		SetFinally<FileMap_G<T> > fin(this);

		if (0 > (_fd = open(path.c_str(), flags, 0664)))
			throw string(MKStr() << "error: open('" << path << "'): "
				     << strerror(errno));

		if (!_size)
		{
			struct stat st;
			if (fstat(_fd, &st))
				throw string(MKStr() << "error: stat('"
					     << path << "'): "
					     << strerror(errno));
			_size = st.st_size;
		}
		else
		{
			if (0 > lseek(_fd, _size-1, SEEK_SET))
				throw string(MKStr()
					     << "error: lseek('" << path << "'): "
					     << strerror(errno));
			if (1 != write(_fd, "", 1))
				throw string(MKStr()
					     << "error: write('" << path << "'): "
					     << strerror(errno));
		}

		int fl = ((flags & O_WRONLY) || (flags & O_RDWR))
			? PROT_WRITE | PROT_READ
			: PROT_READ;
		if (MAP_FAILED == (_addr = (T*)mmap(0, _size,
						    fl,
						    MAP_SHARED,
						    _fd, 0)))
			throw string(MKStr()
				     << "error: map('" << path << "'): "
				     << strerror(errno));

		fin.cancel();
	}
	~FileMap_G() throw() { release();}
	void release() throw()
	{
		if (_addr) { munmap(_addr, _size); _addr = 0; }
		if (_fd) { close(_fd); _fd = 0; }
	}

	inline T *addr() const { return _addr; }
	inline off_t size() const { return _size; }
	inline const string &path() const { return _path; }
};

typedef FileMap_G<fq_t> FileMap;

void chkSystem(int retval)
{
	if (retval < 0)
		throw string("error: fork failed");
	else if (retval)
		throw string("error: command execution failed");

}

int getint(string str)
{
	stringstream ss(str);
	int i;
	ss >> i;
	return i;
}

double tdiff(const struct timeval& t1,
	     const struct timeval& t2)
{
	double retval = 0;
	retval = t2.tv_sec - t1.tv_sec;
	if (t1.tv_usec < t2.tv_usec)
	{
		retval += 1-(t1.tv_usec - t2.tv_usec)*1e-6;
		retval -= 1;
	}
	else
	{
		retval += (t2.tv_usec - t1.tv_usec)*1e-6;
	}
	return retval;
}

char* throughput(off_t size,
		 const struct timeval& t1,
		 const struct timeval& t2)
{
	double s = size;
	static char buf[32];
	sprintf(buf, "%.4fMB/s", s/tdiff(t1, t2)/(1<<20));
	return buf;
}

char* timediff(const struct timeval& t1,
		const struct timeval& t2)
{
	static char buf[32];
	sprintf(buf, "%lf", tdiff(t1, t2));
	return buf;
}

int main(int argc, char **argv)
try
{
	fq::init();

	g_thread_init(0);

	if (argc < 7)
		throw string(MKStr() << "usage: " << argv[0] <<
			     " <input filename> <N> <ncpus> <blocksize> <mode> <id>");

	const string fname = argv[1];
	const int N = getint(argv[2]);
	NCPUS = getint(argv[3]);
	BLOCK_SIZE = getint(argv[4]);
	const string mode = argv[5];
	const string id = argv[6];
	const string &fout = fname + "_out_" + id;
	const string &fdec = fname + "_decoded_" + id;
	const string &fmatr = fname + "_matr_" + id;

	if (!(mode == "c" || mode =="d"))
		throw string("Invalid mode specified.");

	if (mode == "c")
	{
		printf("MEM file=%s mode=c q=%d N=%d CPUs=%d BS=%d ",
		       fname.c_str(), fq_size, N, NCPUS, BLOCK_SIZE);

		off_t fsize;
		auto_arr_ptr<fq_t> m1 = new fq_t [N*N];
		auto_arr_ptr<fq_t> mi;
		auto_arr_ptr<fq_t> mc;

		struct timeval begin_gen, end_gen;
		int sing = 0;

		{
		auto_arr_ptr<fq_t> minv = new fq_t [N*N];
		gettimeofday(&begin_gen, 0);
		do {
			++sing;
			rand_matr(m1, N, N);
		} while (!invert(m1, minv, N, N));
		gettimeofday(&end_gen, 0);
		}

		{
			FileMap infile(fname);
			fsize = infile.size();

			if (fsize % N)
				throw string(MKStr()
					     << "File size (" << fsize
					     << ") is not dividable by block size ("
					     << N << ")");

			mi = new fq_t[fsize];
			mc = new fq_t[fsize];
			memcpy(mi, infile.addr(), fsize);
		}

		struct timeval begin, end;
		gettimeofday(&begin, 0);
		pmul(m1, mi, mc, N, N, fsize/N/sizeof(fq_t));
		gettimeofday(&end, 0);

		printf("matrgen=%s ", timediff(begin_gen, end_gen));
		printf("t=%s ", timediff(begin, end));
		printf("tp=%s\n", throughput(fsize, begin, end));

		FileMap::save(m1, N*N*sizeof(fq_t), fmatr);
		FileMap::save(mc, fsize, fout);

		if (sing > 1)
			printf("# Singular matrices generated: %d\n", sing-1);
	}

	bool singular=false;
	if (mode == "d")
	{
		printf("MEM file=%s mode=d q=%d N=%d CPUs=%d BS=%d ",
		       fname.c_str(), fq_size, N, NCPUS, BLOCK_SIZE);

		off_t fsize;
		auto_arr_ptr<fq_t> m1 = new fq_t [N*N];
		auto_arr_ptr<fq_t> minv = new fq_t [N*N];
		auto_arr_ptr<fq_t> mi;
		auto_arr_ptr<fq_t> md;

		{
			FileMap matr(fmatr);
			memcpy(m1, matr.addr(), N*N*sizeof(fq_t));
		}
		{
			FileMap infile(fout);
			fsize = infile.size();

			if (fsize % N)
				throw string(MKStr()
					     << "File size (" << fsize
					     << ") is not dividable by block size ("
					     << N << ")");

			mi = new fq_t[fsize];
			md = new fq_t[fsize];
			memcpy(mi, infile.addr(), fsize);
		}

		struct timeval begin_inv, end_inv;
		gettimeofday(&begin_inv, 0);
		if (!invert(m1, minv, N, N))
		{
			singular=true;
			goto __break;
		}
		gettimeofday(&end_inv, 0);

		struct timeval begin, end;
		gettimeofday(&begin, 0);
		pmul(minv, mi, md, N, N, fsize/N/sizeof(fq_t));
		gettimeofday(&end, 0);

		printf("matrinv=%s ", timediff(begin_inv, end_inv));
		printf("t=%s ", timediff(begin, end));
		printf("tp=%s\n", throughput(fsize, begin, end));

		FileMap::save(md, fsize, fdec);
	}

__break:
	if (singular)
		throw string("Generated matrix was singular.");

	return 0;
}
catch (const string &msg)
{
	cerr << "Error: " << msg << endl;
}
