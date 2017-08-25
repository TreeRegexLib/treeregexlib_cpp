#ifndef TIMEIT_H
#define TIMEIT_H
#include <iostream>
#include <chrono>

template<typename T>
class _timeit {
	std::chrono::time_point<T> last_measured;
	std::chrono::time_point<T> start;
public:
	_timeit():start(T::now()){
		last_measured = start;
	}

	void end(const std::string& msg){
		auto current = T::now();
		std::cerr << msg << ": " << std::chrono::duration <double> (current-last_measured).count() << " seconds\n";
		last_measured = current;
	}

	~_timeit(){
		std::cerr << "Total time: "<< std::chrono::duration <double> (T::now()-start).count() << " seconds\n";
	}
};

typedef _timeit<std::chrono::high_resolution_clock> timeit;

#endif /* TIMEIT_H */
