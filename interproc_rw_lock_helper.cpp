#include "interproc_rw_lock_helper.h"

#ifdef max
#undef max
#endif

namespace tepp
{
	bool interproc_rw_lock_helper::is_ok(bool * c)
	{
		return !c || !*c;
	}
	bool interproc_rw_lock_helper::is_ok(e_try_get_lock_or_cancel e)
	{
		return e == e_try_get_lock_or_cancel::ok;
	}
	bool interproc_rw_lock_helper::is_ok(e_try_get_lock_infinite e)
	{
		return e == e_try_get_lock_infinite::ok;
	}

	bool interproc_rw_lock_helper::is_ok(e_try_get_lock e)
	{
		return e == e_try_get_lock::ok;
	}
	
	e_try_get_lock_or_cancel interproc_rw_lock_helper::try_get_lock_infinite(HANDLE & h, ulong_t wait_time, bool * c, std::string * err_msg)
	{
		return try_get_lock(h, wait_time, c, std::numeric_limits<int64_t>::max(), err_msg);
	}
	e_try_get_lock interproc_rw_lock_helper::try_get_lock_immediate(HANDLE & h, std::string * err_msg)
	{
		return (e_try_get_lock)try_get_lock(h, 0, nullptr, 1, err_msg);
	}

	std::string interproc_rw_lock_helper::get_last_error()
	{
		//Get the error message, if any.
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0)
			return std::string(); //No error message has been recorded

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;
	}
	e_try_get_lock_or_cancel interproc_rw_lock_helper::try_get_lock(HANDLE & h, ulong_t wait_time, bool * c, uint64_t max_tries, std::string * err_msg)
	{
		while (1)
		{
			if (max_tries == 0) return e_try_get_lock_or_cancel::no_more_tries;
			if (!is_ok(c)) return e_try_get_lock_or_cancel::user_cancel;

			auto er = WaitForSingleObject(h, wait_time);

			switch (er)
			{
			case WAIT_ABANDONED:
			case WAIT_OBJECT_0:
				return e_try_get_lock_or_cancel::ok;

			case WAIT_TIMEOUT:
				--max_tries;
				continue;
			case WAIT_FAILED:

				if (err_msg)
					err_msg->append("|").append(get_last_error());
				return e_try_get_lock_or_cancel::error;
			}
		}
	}
	e_try_get_lock_infinite interproc_rw_lock_helper::try_get_lock_all_infinite(HANDLE * h, ulong_t count, ulong_t wait_time, bool * c, std::string * err_msg)
	{
		return try_get_lock_multiple_infinite(h, count, true, wait_time, c, err_msg, nullptr);
	 }
	e_try_get_lock_infinite interproc_rw_lock_helper::try_get_lock_one_of_infinite(HANDLE * h, ulong_t count, ulong_t wait_time, bool * c, std::string * err_msg, int & locked_index)
	{
		return try_get_lock_multiple_infinite(h, count, false, wait_time, c, err_msg, &locked_index);
	}

	e_try_get_lock_infinite interproc_rw_lock_helper::try_get_lock_multiple_infinite(HANDLE * h, ulong_t count, bool all, ulong_t wait_time, bool * c, std::string * err_msg, int * locked_index)
	{
		while (1)
		{
			if (!is_ok(c)) return e_try_get_lock_infinite::user_cancel;

			auto er = WaitForMultipleObjects(count, h, all, wait_time);

			if (er == WAIT_TIMEOUT)
			{
				continue;
			}

			if (er == WAIT_FAILED)
			{
				if (err_msg)
					err_msg->append("|").append(get_last_error());
				return e_try_get_lock_infinite::error;
			}

			if (er >= WAIT_OBJECT_0 && er <= WAIT_OBJECT_0 + count - 1)
			{
				if(locked_index )
					*locked_index = er - WAIT_OBJECT_0;
				return e_try_get_lock_infinite::ok;
			}

			if (er >= WAIT_ABANDONED_0 && er <= WAIT_ABANDONED_0 + count - 1)
			{
				if (locked_index)
					*locked_index = er - WAIT_ABANDONED_0;
				return e_try_get_lock_infinite::ok;
			}

		}
	}
}
