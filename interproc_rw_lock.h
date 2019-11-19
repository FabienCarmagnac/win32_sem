#pragma once

#include "interproc_rw_lock_helper.h"
#include <vector>
#include <atomic>

namespace tepp
{

	class interproc_rw_lock : interproc_rw_lock_helper
	{
		std::vector<HANDLE> _r;
		HANDLE  _w;
			   
		interproc_rw_lock(HANDLE w, std::vector<HANDLE> && rconst) noexcept(true);
		interproc_rw_lock() = delete;
		interproc_rw_lock(const interproc_rw_lock &) = delete;

	public:
		static const ulong_t wait_ms = 250;


		/*
		@param name a unique id machine wide used amongst processes/threads.  Must be non empty and ideally non trivial.
		@param max_readers maximum readers in parallel. Must be > 0 and < 64 . For a given 'name', must be the same value accross threads/processes sharing this 'name'.
		@return the interproc_rw_lock instance or nullptr if an error occured. 

		*/

		static interproc_rw_lock* try_create_interproc_rw_lock(const std::string & name, int max_readers) noexcept(true);

		/* Allow to execute code with 'read' role, eg may be in parallel with other readers process/thread but guaranteed with no writer.
		For this, it acquires a writer-mutex, then one available reader mutex, then free writer-mutex, then invokes f, then release read-mutex. 
		@param f the lambda to invoke. Must now throw.
		@param cancel a cancellation token which can be used to interrupt trying to acquire the mutexes. Other can execute: *cancel=true. Can be null.
		@param pr_wait_ms maximum time to try to get the read lock, in milliseconds. Can be 0.
		@param pr_wait_ms maximum time to try to get the write lock, in milliseconds. Can be 0.
		@param err_msg if an error occurs while trying to acquire a lock, then the error is appended to this string (if not null). Can be null.
		@return bool true if f has been invok, false either.
		*/
		bool read_under_lock(action0 f, std::atomic_bool * cancel = nullptr, ulong_t pr_wait_ms = wait_ms, ulong_t pw_wait_ms = wait_ms, std::string * err_msg = nullptr) noexcept(true);

		/* Allow to execute code with exclusive 'write' role, eg no other writer or reader process/thread can execute in parallel.
		For this, it acquires a writer-mutex, then all reader-mutexes, then invokes f, then free all reader-mutexes, then release write-mutex.
		@param f the lambda to invoke. Must now throw.
		@param cancel a cancellation token which can be used to stop trying to acquire the locks(s). Other can execute: *cancel=true. Can be null.
		@param pr_wait_ms maximum time to try to get the read lock, in milliseconds. Can be 0.
		@param pr_wait_ms maximum time to try to get the write lock, in milliseconds. Can be 0.
		@param err_msg if an error occurs while trying to acquire a lock, then the error is appended to this string (if not null). Can be null.
		@return bool true if f has been invok, false either.
		*/
		bool write_under_lock(action0 f, std::atomic_bool * cancel = nullptr, ulong_t pr_wait_ms = wait_ms, ulong_t pw_wait_ms = wait_ms, std::string * err_msg = nullptr) noexcept(true);

	};


}