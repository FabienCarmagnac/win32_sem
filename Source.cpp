#include <windows.h>
#include <thread>
#include <chrono>
#include <string>
#include <iostream>
#include <functional>
#include <conio.h>

#ifdef max
#undef max
#endif

using namespace std;

std::string GetLastErrorAsString()
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

typedef unsigned long ulong_t;
typedef std::function<void()> action0;
typedef std::function<bool()> pred0;

#define COUT(x) cout << x << endl

class interproc_rw_lock
{
	HANDLE  _w;
	HANDLE  _r;
	const int _max_readers;
	SECURITY_ATTRIBUTES _sa;

	static bool is_ok(bool * cancel)
	{
		return !cancel || !*cancel;
	}

	static bool release_lock(HANDLE & h, long inc, std::string * err_msg)
	{
		COUT(&h << " releasing");
		bool r = ReleaseSemaphore(h, inc, nullptr);
		if (!r && err_msg)
			err_msg->append("|").append(GetLastErrorAsString());
		return r;
	}


	enum class e_try_get_lock
	{
		ok, no_more_tries, error, user_cancel
	};

	bool is_ok(e_try_get_lock e)
	{
		return e == e_try_get_lock::ok;
	}

	struct scope_guard
	{
		action0 a;
		scope_guard(action0 pa) :a{ pa } {}
		~scope_guard()
		{
			a();
		}
	};
	static e_try_get_lock try_get_lock_infinite(HANDLE & h, ulong_t wait_time, bool * cancel, std::string * err_msg)
	{
		return try_get_lock(h, wait_time, cancel, std::numeric_limits<int64_t>::max(), err_msg);
	}
	static e_try_get_lock try_get_lock(HANDLE & h, ulong_t wait_time, bool * c, uint64_t max_tries, std::string * err_msg)
	{
		while (1)
		{
			if (max_tries == 0) return e_try_get_lock::no_more_tries;
			if (!is_ok(c)) return e_try_get_lock::user_cancel;

			COUT(&h << " waiting");
			auto er = WaitForSingleObject(h, wait_time);

			switch (er)
			{
			case WAIT_ABANDONED:
				COUT(&h << " AB");
			case WAIT_OBJECT_0:
				COUT(&h << " lock ok");
				return e_try_get_lock::ok;

			case WAIT_TIMEOUT:
				COUT(&h << " timedout");
				--max_tries;
				continue;
			case WAIT_FAILED:
				COUT(&h << " failed");

				if (err_msg)
					err_msg->append("|").append(GetLastErrorAsString());
				return e_try_get_lock::error;
			}
		}
	}

public:

	static const ulong_t w_wait_ms = 250;
	static const ulong_t r_wait_ms = 250;

	interproc_rw_lock(const std::string & name, int max_readers)
		:_max_readers(max_readers)
	{
		_sa.bInheritHandle = false;
		_sa.nLength = 0;
		_sa.lpSecurityDescriptor = NULL;


		_w = CreateSemaphore(&_sa, 1, 1, ("Global\\w_" + name).c_str());
		_r = CreateSemaphore(&_sa, max_readers, max_readers, ("Global\\r_" + name).c_str());

		COUT(&_r << " creating r");
		COUT(&_w << " creating w");

		if (!_w || !_r)
		{
			std::cerr << "cant create semaphore : " << GetLastError() << "\n";
		}
	}

	bool is_valid()const
	{
		return _w && _r;
	}
	bool read_under_lock(action0 f, bool * cancel = nullptr, ulong_t pr_wait_ms = r_wait_ms, ulong_t pw_wait_ms = w_wait_ms, std::string * err_msg = nullptr)
	{

		if (!is_ok(try_get_lock_infinite(_w, pw_wait_ms, cancel, err_msg)))
			return false;

		bool tmp = is_ok(try_get_lock_infinite(_r, pr_wait_ms, cancel, err_msg));
		release_lock(_w, 1, err_msg);

		if(!tmp)
			return false;

		f();

		release_lock(_r, 1, err_msg);
		return true;
	}


	bool write_under_lock(action0 f, bool * c, int pr_wait_ms, int pw_wait_ms , std::string * err_msg)
	{
		if (!is_ok(try_get_lock_infinite(_w, pw_wait_ms, c, err_msg)))
			return false;

		bool ret = false;
		int acquired = 0;
		int rwt = 0;

		scope_guard sg([&]
		{
			while (acquired--)
			{
				release_lock(_r, 1, err_msg);
			}
			release_lock(_w, 1, err_msg);
		});

		while (is_ok(c) && acquired != _max_readers)
		{

			//try to get fast
			auto e = try_get_lock(_r, rwt, c, 1, err_msg);
			switch (e)
			{
			case e_try_get_lock::error:
			case e_try_get_lock::user_cancel:
				return false;

			case e_try_get_lock::ok:
				++acquired;
				continue;

			case e_try_get_lock::no_more_tries:
				if (rwt == 0)
					rwt = pr_wait_ms;
				continue;
			};
		}

		// confirm we got them all
		if (is_ok(c) && e_try_get_lock::no_more_tries == try_get_lock(_r, 0, c, 1, err_msg))
		{
			f();
			return true;
		}
		return false;
	}
};


void ww()
{
	cout << ">>> w\n";
	this_thread::sleep_for(std::chrono::seconds(1));
	cout << '.';
	this_thread::sleep_for(std::chrono::seconds(1));
	cout << '.';
	this_thread::sleep_for(std::chrono::seconds(1));
	cout << '.';
	this_thread::sleep_for(std::chrono::seconds(1));
	cout << '.';
	this_thread::sleep_for(std::chrono::seconds(1));
	throw 1;
	cout << "<<< w\n";
}
void rr()
{
	cout << ">>> r\n";
	this_thread::sleep_for(std::chrono::seconds(1));
	cout << "<<< r\n";

}
int main(void)
{
	interproc_rw_lock il("toto", 2);

	cout << "press r, w or . to leave\n";
	string er;
	while (1)
	{
		char c = (char)_getch();
		switch (c)
		{
		case '.': 
			break;

		case 'w':
			cout << "\nwriting\n";
			if (!il.write_under_lock([] {ww(); }, nullptr, 10, 10, &er))
				cout << "err w:" << er << "\n";
			continue;

		case 'r':
			cout << "\nreading\n";
		if (!il.read_under_lock([] {rr(); }, nullptr, 10, 10, &er))
				cout << "err r:" << er << "\n";
		continue;


		};
	}

	
	cout << "bye\n";

}
