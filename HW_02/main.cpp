#include "gtu.cpp"

gtu::mutex s1;
gtu::mutex s2;

int c = 0;

condition_variable cond_values; 
mutex stdMutex;

#ifdef __linux__
	int secret_num; // policy
	struct sched_param parameters_schedule; // struct sched_param sched_p;
#endif

bool control_condition = false;

bool cond_control() { return control_condition; }


void parameters_from_schedule(std::thread & t, int pro)
{
	#ifdef __linux__ 
		sched_param params_schedule;
		params_schedule.sched_priority = pro;
		if (pthread_setschedparam(t.native_handle(), SCHED_FIFO, &params_schedule))
			cout << "Thread kayit edilmedi." << endl;
	#elif _WIN32
		/*if (SetThreadPriority(&t, pro))
		{
			cout << "Thread kayit edilmedi." << endl;
		}*/
	#endif
	
	s1.saveThread(t);
	s2.saveThread(t);

}

void thread_func_A(int task_priority)
{
	cout << "Thread " <<  task_priority << " A fonksiyonunu cagirdi. " << endl;

	std::unique_lock<std::mutex> cond_variable_lock(stdMutex);
	cond_values.wait(cond_variable_lock,cond_control);

	cout << "Thread " << task_priority << " : A fonksiyonunu bloke etti." << endl;

	std::lock_guard<gtu::mutex> locks1(s1);
	std::lock_guard<gtu::mutex> locks2(s2);
	c+=1;
	// unix
	#ifdef __linux__ 
		pthread_getschedparam(pthread_self(), &secret_num, &parameters_schedule);
		cout << "Priority:\t" << parameters_schedule.sched_priority << endl;
	#elif _WIN32
		//cout << "Priority:\t" << GetThreadPriority(GetCurrentThread()) << endl;
	#endif
}

void thread_func_B(int task_priority)
{
	cout << "Thread " <<  task_priority << " B fonksiyonunu cagirdi. " << endl;

	std::unique_lock<std::mutex> cond_variable_lock(stdMutex);
	cond_values.wait(cond_variable_lock, cond_control);

	cout << "Thread " << task_priority << " B fonksiyonunu bloke etti." << endl;

	std::lock_guard<gtu::mutex> locks1(s2);
	std::lock_guard<gtu::mutex> locks2(s1);
	c+=1;
	// unix
	#ifdef __linux__ 
		pthread_getschedparam(pthread_self(), &secret_num, &parameters_schedule);
		cout << "Priority:\t" << parameters_schedule.sched_priority << endl;
	#elif _WIN32
		//cout << "Priority:\t" << GetThreadPriority(GetCurrentThread()) << endl;
	#endif
}



int main(int argc, char const *argv[])
{
	std::thread t1(thread_func_A,1);
	std::thread t2(thread_func_B,2);
	std::thread t3(thread_func_B,3);
	std::thread t4(thread_func_A,4);
	std::thread t5(thread_func_B,5);


	parameters_from_schedule(t1, 20);
	parameters_from_schedule(t2, 19);
	parameters_from_schedule(t3, 22);
	parameters_from_schedule(t4, 15);
	parameters_from_schedule(t5, 17);


	control_condition = true;
	cond_values.notify_all();

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();

	return 0;
}
