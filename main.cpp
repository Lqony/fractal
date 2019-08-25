#include <iostream>
#include <string>
#include <unordered_set>
#include "node.h"

std::vector<std::shared_ptr<node>> nodes;
std::list<hashcoin*> g_coins;

int main(int argc, char **argv) 
{
    /*
	string shex;
	HexEncoder hex(new StringSink(shex));

    */
	set_seed(64); 
	
	std::random_device rd;
	std::mt19937 gen(rd());
	
	std::cout << "1" << std::endl;
	std::list<my_hashcoin> coins;
	size_t val = 10;
	while(coins.size() < 50)
	{
		std::string prv_key1;
		std::string pub_key1;

		std::string prv_key2;
		std::string pub_key2;

		get_crypt_keys(prv_key1, pub_key1);
		get_sign_keys(prv_key2, pub_key2);

		coins.emplace_back(val, pub_key1, pub_key2, prv_key1, prv_key2);
		g_coins.emplace_back(&coins.back());
		
		if(coins.size() % 5000 == 0)
			std::cout << coins.size() << std::endl;
	}
	std::cout << "2" << std::endl;
	auto it = coins.begin();
	for(int i = 0; i < 50; i++)
	{
		nodes.push_back(std::make_shared<node>(*it));
		it++;
	}
	std::cout << "3" << std::endl;
	std::uniform_int_distribution<uint64_t> dis(0, nodes.size()-1);
	for(const auto& a : nodes)
	{
		while(a->can_connect())
		{
			const auto& af = nodes.at(dis(gen));
			
			if(&(*a) == &(*af))
				continue;

			if(af->can_connect())
			{
				af->connect(a);
				a->connect(af);
			}
		}
	}
	std::cout << "4" << std::endl;
	for(const auto& a : nodes)
	{
		while(a->can_connect())
		{
			std::cout << "ERROR!" << std::endl;
		}
	}
	std::cout << "5" << std::endl;
	const auto& chosen_one = nodes.at(dis(gen));
	
	chosen_one->test_message();


	std::cout << "XXX" << std::endl;

	return 0;
}