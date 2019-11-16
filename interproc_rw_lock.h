#pragma once

#include "interproc_rw_lock_helper.h"
#include <vector>

namespace tepp
{

	class interproc_rw_lock : interproc_rw_lock_helper
	{
		std::vector<HANDLE> _r;
		HANDLE  _w;
			   
		interproc_rw_lock(HANDLE w, std::vector<HANDLE> && rconst);
		interproc_rw_lock() = delete;
		interproc_rw_lock(const interproc_rw_lock &) = delete;

	public:
		static const ulong_t wait_ms = 250;
		static const ulong_t r_wait_ms = 250;

		static interproc_rw_lock* try_create_interproc_rw_lock(const std::string & name, int max_readers);

		bool read_under_lock(action0 f, bool * cancel = nullptr, ulong_t pr_wait_ms = wait_ms, ulong_t pw_wait_ms = wait_ms, std::string * err_msg = nullptr);
		bool write_under_lock(action0 f, bool * cancel = nullptr, ulong_t pr_wait_ms = wait_ms, ulong_t pw_wait_ms = wait_ms, std::string * err_msg = nullptr);

	};


}