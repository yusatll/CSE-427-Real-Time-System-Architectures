#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <vector>
#include <condition_variable>

#ifdef _WIN32 
#include <windows.h>
#include <tchar.h>
#endif

using namespace std;

namespace gtu 
{
	class mutex;

	class thread_save_class 
	{
	public:
		thread::id save_ID;		
		std::vector<gtu::mutex*> th_save_mutex_vec; 	
		int Priority;
	};


	// MUTEX CLASS EXTEND FROM STD::MUTEX
	class mutex : public std::mutex
	{

	private:
		int ceiling; 	
		std::vector<thread_save_class> th_save_vec; 	
		std::mutex stdMutex; 	
		bool acq; 	
		condition_variable cond_variable; 

	public:

		std::function<bool()> condition_variable_acq_control = [&]
		{
			gtu::mutex* m = mutex_acq_control();
			if (m == nullptr)
				return true;
			return false;
		};
		
		bool try_lock()	{ return get_acq(); }

		void unlock()
		{
			
			int i = 0;
			while(i < th_save_vec.size())
			{
				auto t = th_save_vec[i];
				if (t.save_ID == this_thread::get_id())
				{
					th_save_vec.erase(th_save_vec.begin() + i);
					break;
				}
				i++;
			}

			this->set_ceiling(max_priority(th_save_vec));
			cond_variable.notify_one();
			cout << "Mutex unlock."<< endl;
			std::mutex::unlock();
			acq = false;
		}


		int max_priority(std::vector<gtu::thread_save_class> v) 
		{
			int max = -1000000000;
			int i = 0;
			while(i < v.size())
			{
				auto pr = v[i];
				if(pr.Priority > max)
					max = pr.Priority;
				i++;
			}
			return max;
		}

		gtu::mutex * mutex_acq_control() 
		{
			int i = 0;
			while(i < th_save_vec.size())
			{
				auto m = th_save_vec[i];
				if(m.save_ID == std::this_thread::get_id())
				{
					int j = 0;
					while(j < m.th_save_mutex_vec.size())
					{
						gtu::mutex* t = m.th_save_mutex_vec[j];
						if((t)->get_acq())
							return t;
						j++;
					}
				}
				i++;
			}
		}

		bool thread_run_control(std::thread::id this_thread_id) 
		{
			int i = 0;
			while(i < th_save_vec.size())
			{
				auto th_id = th_save_vec[i];
				if (th_id.save_ID == this_thread_id)
					return true;
				i++;
			}
			return false;
		}

		void lock()
		{
			
			if (thread_run_control(this_thread::get_id()))
			{
				gtu::mutex* mutexptr = mutex_acq_control();

				if (mutexptr == nullptr)
				{
					cout << "Mutex lock."<< endl;
					std::mutex::lock();
					acq = true;
				}
				else
				{
					if (get_saved_thread_priority() > mutexptr->get_ceiling())
					{
						cout << "Mutex lock."<< endl;
						std::mutex::lock();
						acq = true;
					}
					else
					{
						std::unique_lock<std::mutex> lock1(stdMutex);
						cond_variable.wait(lock1, condition_variable_acq_control);
						cout << "Mutex lock."<< endl;
						std::mutex::lock();
						acq = true;
					}
				}
			}
			else{
				cout << "Thread onceden register edilmemis." << endl;

			}

		}

		void saveThread(thread & save_this) 
		{
			gtu::thread_save_class t_save;

			t_save.save_ID = save_this.get_id();

			//struct sched_param parameters_struct_from_schedule;
			// unix
			#ifdef __linux__ 
				int p;
				struct sched_param parameters_struct_from_schedule;
				pthread_getschedparam(pthread_self(), &p, &parameters_struct_from_schedule);
				t_save.Priority = parameters_struct_from_schedule.sched_priority;
			#elif _WIN32				
				//SetThreadPriority(GetCurrentThread, GetThreadPriority(&save_this));
			#endif
			//pthread_getschedparam(pthread_self(), &p, &parameters_struct_from_schedule);

			//t_save.Priority = parameters_struct_from_schedule.sched_priority;

			t_save.th_save_mutex_vec.push_back(this);

			th_save_vec.push_back(t_save);

			this->set_ceiling(max_priority(th_save_vec));

		}

		int get_saved_thread_priority()
		{
			int i = 0;
			while(i < th_save_vec.size())
			{
				auto th_id = th_save_vec[i];
				if (th_id.save_ID == std::this_thread::get_id())
				{
					return th_id.Priority;
				}
				i++;
			}
			return -1;
		}


		int get_ceiling() { return this->ceiling; }
		void set_ceiling(const int c) { ceiling = c; }

		bool get_acq() { return this->acq; }
		void set_acq(const bool a) { acq = a; }
	}; 
}