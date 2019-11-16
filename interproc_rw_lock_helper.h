#pragma once

#include <windows.h>
#include <string>
#include <atomic>
#include <functional>


namespace tepp
{
	typedef unsigned long ulong_t;
	typedef std::function<void()> action0;

	enum class e_try_get_lock_or_cancel
	{
		ok, no_more_tries, error, user_cancel
	};

	enum class e_try_get_lock
	{
		ok = (int)e_try_get_lock_or_cancel::ok,
		no_more_tries = (int)e_try_get_lock_or_cancel::no_more_tries,
		error = (int)e_try_get_lock_or_cancel::error,
		
	};

	enum class e_try_get_lock_infinite
	{
		ok = (int)e_try_get_lock_or_cancel::ok,
		error = (int)e_try_get_lock_or_cancel::error,
		user_cancel= (int)e_try_get_lock_or_cancel::user_cancel
	};


	class interproc_rw_lock_helper
	{
	private:
		static e_try_get_lock_infinite try_get_lock_multiple_infinite(HANDLE * h, ulong_t count, bool all, ulong_t wait_time, std::atomic_bool *  c, std::string * err_msg, int * locked_index);
		static e_try_get_lock_or_cancel try_get_lock(HANDLE & h, ulong_t wait_time, std::atomic_bool *  c, uint64_t max_tries, std::string * err_msg);

	protected:
		static std::string get_last_error();

		static bool is_ok(std::atomic_bool *  cancel);
		static bool is_ok(e_try_get_lock_or_cancel e);
		static bool is_ok(e_try_get_lock_infinite e);
		static bool is_ok(e_try_get_lock e);

		static e_try_get_lock_or_cancel try_get_lock_multi(HANDLE & h, ulong_t wait_time, std::atomic_bool *  c, uint64_t max_tries, std::string * err_msg);

		static e_try_get_lock_or_cancel try_get_lock_infinite(HANDLE & h, ulong_t wait_time, std::atomic_bool *  c, std::string * err_msg);
		static e_try_get_lock try_get_lock_immediate(HANDLE & h, std::string * err_msg);
		static e_try_get_lock_infinite try_get_lock_all_infinite(HANDLE * h, ulong_t count, ulong_t wait_time, std::atomic_bool *  c, std::string * err_msg);
		static e_try_get_lock_infinite try_get_lock_one_of_infinite(HANDLE * h, ulong_t count, ulong_t wait_time, std::atomic_bool *  c, std::string * err_msg, int & locked_index);

	};


}