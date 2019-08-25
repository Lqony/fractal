//The man holding all the mony in the world
#include <unordered_map>
#include <mutex>
#include "hashcoin.h"

//STORES COINS AND PROCESS THEIR VALUE

class the_man
{
public:
  
	inline static the_man& inst()
	{
		static the_man the_man_inst;
		return the_man_inst;
	}
	
	inline static const the_man& inst()
	{
		static const the_man& the_man_inst = the_man::inst();
		return the_man_inst;
	}
  
private:
	the_man() {}
	
	struct pocket
	{
		std::mutex mtx;
		std::unordered_map<hash_type, hashcoin> coins;
	};
};