#include "interproc_rw_lock.h"
#include <functional>


namespace tepp
{
	struct scope_guard
	{
		std::vector<action0> a;
		~scope_guard()
		{
			for (auto & f : a)
				f();
		}
	};

	interproc_rw_lock::interproc_rw_lock(HANDLE w, std::vector<HANDLE> && r)
		: _w(w)
		, _r(std::move(r))
	{
	}

	interproc_rw_lock* interproc_rw_lock::try_create_interproc_rw_lock(const std::string & name, int max_readers)
	{
		if (max_readers >= 64)
			return nullptr;

		SECURITY_ATTRIBUTES sa;
		sa.bInheritHandle = false;
		sa.nLength = 0;
		sa.lpSecurityDescriptor = NULL;

		HANDLE w = CreateMutex(&sa, false, ("Global\\w_" + name).c_str());
		if (!w) return nullptr;

		scope_guard sg;
		sg.a.push_back([w] { CloseHandle(w); });

		std::vector<HANDLE> r;
		r.resize(max_readers, 0);
		std::string prefix = "Global\\r_" + name;
		for (int i = 0; i < max_readers; ++i)
		{
			auto n  = prefix + std::to_string(i);
			HANDLE rh = CreateMutex(&sa, false, n.c_str());
			if (!rh)
				return nullptr;
			r[i] = rh;
			sg.a.push_back([rh] { CloseHandle(rh); });
		}

		// all good
		sg.a.clear();
		return new interproc_rw_lock(w, std::move(r));

	}

	bool interproc_rw_lock::read_under_lock(action0 f, bool * c, ulong_t pr_wait_ms, ulong_t pw_wait_ms , std::string * err_msg )
	{
		
		if (!is_ok(try_get_lock_infinite(_w, pw_wait_ms, c, err_msg)))
			return false;

		int i;
		bool tmp = is_ok(try_get_lock_one_of_infinite(_r.data(), (ulong_t)_r.size(), pr_wait_ms, c, err_msg, i));
		ReleaseMutex(_w);

		if (!tmp)
			return false;

		f();

		ReleaseMutex(_r[i]);
		return true;
	}

	bool interproc_rw_lock::write_under_lock(action0 f, bool * c, ulong_t pr_wait_ms, ulong_t pw_wait_ms, std::string * err_msg)
	{
		if (!is_ok(try_get_lock_infinite(_w, pw_wait_ms, c, err_msg)))
			return false;

		bool tmp = is_ok(try_get_lock_all_infinite(_r.data(),(ulong_t)_r.size(), pr_wait_ms, c, err_msg));
		if (!tmp)
		{
			ReleaseMutex(_w);
			return false;
		}

		f();

		for (auto & h : _r)
		{
			ReleaseMutex(h);
		}

		ReleaseMutex(_w);

		return true;
	}

}