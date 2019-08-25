#pragma once

#include <mutex>
#include <list>
#include <vector>
#include <set>
#include <thread>
#include <atomic>
#include <memory>
#include <algorithm>
#include <functional>

#include "serialize.h"
#include "crypto.h"
#include "hashcoin.h"

extern std::list<hashcoin*> g_coins;

class node;

extern std::vector<std::shared_ptr<node>> nodes;

class node
{
public:
  
	node(my_hashcoin& coin) : gen(rd()), running(false), sent_msg(false), coin(coin)  
	{
		std::uniform_int_distribution<uint64_t> dis(1, UINT64_MAX);
		id = dis(gen);
	}
	
	~node()
	{
		running = false;
		if(main_thd.joinable())
			main_thd.join();		
	}
	
	uint64_t get_id() { return id; }
	
	inline void run()
	{
		running = true;
		main_thd = std::thread(&node::main_thread, this);
	}
	
	inline const hashcoin& get_coin() { return const_cast<const my_hashcoin&>(coin); }
	
	inline void pass_encrypted(std::string enc_bin)
	{  
		if(!enc_recv.insert(enc_bin).second)
			return;
		
		if(!try_decrypt(enc_bin))
		{
			for(const auto& a : connectinos)
				a->pass_encrypted(enc_bin);
		}
	}
	
	inline void pass_decrypted(part_out recv_part_out)
	{  
		if(recv_part_out.valid == PART_OUT_VALID)
		{
			for(const auto& a : part_outs)
			{
				if(a.part == recv_part_out.part)
					return;
			}

			part_outs.push_back(recv_part_out);
			
			if(part_outs.size() == 5)
			{
				std::cout << "GOT 5 PARTS!" << std::endl;
				
				message msg;
				reconstruct(part_outs, msg);
				
				std::cout << "ID: " << msg.id << std::endl;
				
				transaction tx;
				msg.get_message(tx);
				
				std::cout << "VALUE: " << tx.value << std::endl;
				
				for(auto a : g_coins)
				{
					std::cout << "VERYFIED : " << verify(a->get_veryfier(), msg.blob, msg.signature) << std::endl; 
				}
			}
			else
			{
				for(const auto& a : connectinos)
					a->pass_decrypted(recv_part_out);
			}
		}
	}
	
	inline void test_message()
	{
		std::cout << "SENDING MESSAGE ID: " << get_id() << std::endl;
		message msg;
		msg.id = get_id();
		transaction tx;
		tx.value = 3;
		msg.fill_message(tx);
		
		sign(coin.get_signer(), msg.blob, msg.signature);
		
		std::string msg_bin;
		serialize_to_blob(msg, msg_bin);
		
		std::vector<part_out> part_outs;
		split_into(msg_bin, connectinos.size(), part_outs);
		
		std::shuffle(part_outs.begin(), part_outs.end(), gen);
		
		std::uniform_int_distribution<uint64_t> dis(0, nodes.size()-1);
		std::vector<std::string> part_bin;
		for(const auto& a : part_outs)
		{
			part_bin.emplace_back();
			serialize_to_blob(a, part_bin.back());
			
			//std::string shex;
			//StringSource (part_bin.back(), true, new HexEncoder(new StringSink(shex)));
				
			//std::cout << shex << std::endl << std::endl;*/
		}
		
		std::vector<std::string> enc_part_bin;
		for(const auto& a : part_bin)
		{
			enc_part_bin.emplace_back();
			
			while(true)
			{
				const auto& af = nodes.at(dis(gen));
				
				if(this == af.get())
					continue;

				encrypt(af->get_coin().get_encryptor(), a, enc_part_bin.back());
				break;
			}

			/*std::string shex;
			StringSource (enc_part_bin.back(), true, new HexEncoder(new StringSink(shex)));
			
			std::cout << shex << std::endl << std::endl;*/
		}
		
		auto pit = enc_part_bin.begin();
		auto cit = connectinos.begin();
		while(pit != enc_part_bin.end())
		{
			(*cit)->pass_encrypted(*pit);
			enc_recv.insert(*pit);
			pit++;
			cit++;
		}
		
		/*std::vector<std::string> dec_part_bin;
		for(const auto& a : enc_part_bin)
		{
			dec_part_bin.emplace_back();
			decrypt(pr_key_string, a, dec_part_bin.back());
		}
		
		std::vector<part_out> recv_part_outs;
		for(const auto& a : dec_part_bin)
		{
			recv_part_outs.emplace_back();
			deserialize_from_str(a, recv_part_outs.back());
		}
		
		message msg2;
		reconstruct(recv_part_outs, msg2);
		
		std::cout << msg2.id << " / " << get_id() << std::endl;
		std::cout << msg2.control_data << std::endl;*/
	      
		return;
	}
	
	bool connect(std::shared_ptr<node> conn)
	{
		std::unique_lock<std::mutex> lck(conn_mtx);
		
		if(connectinos.size() >= 5)
			return false;
		
		connectinos.emplace_back(conn);
		
		return true;
	}
	
	bool can_connect()
	{
		std::unique_lock<std::mutex> lck(conn_mtx);
		return connectinos.size() < 5;
	}
	
	std::atomic<bool> sent_msg;
  
private:
  
	struct message
	{
		message() {}

		uint64_t id;
		blob_type blob;
		signature_type signature;
		
		template<class Archive>
		void serialize(Archive &ar, const unsigned int file_version)
		{
			ar & id;
			ar & blob;
			ar & signature;
		}
		
		template<typename T>
		inline void fill_message(T obj)
		{
			serialize_to_blob(obj, blob);
		}
		
		template<typename T>
		inline void get_message(T& obj)
		{
			deserialize_from_blob(blob, obj);
		}
	};
	
	struct out
	{
		uint16_t order;
		std::string blob;
		
		template<class Archive>
		void serialize(Archive &ar, const unsigned int file_version)
		{
			ar & order;
			ar & blob;
		}
	};
	
	inline bool try_decrypt(std::string enc_bin)
	{  
		try
		{
			std::string dec_part_bin;
			decrypt(coin.get_decryptor(), enc_bin, dec_part_bin);
			
			part_out recv_part_out;
			deserialize_from_blob(dec_part_bin, recv_part_out);
			
			if(recv_part_out.valid == PART_OUT_VALID)
			{
				pass_decrypted(recv_part_out);
				return true;
			}
		}
		catch(...){}
		
		return false;
	}

	std::atomic<bool> running;
	
	std::thread main_thd;
	void main_thread();
    
	std::random_device rd;
	std::mt19937 gen;
	
	std::atomic<uint64_t> id;
	
	my_hashcoin& coin;
	
	std::set<std::string> enc_recv;
	std::vector<part_out> part_outs;
	
	std::mutex conn_mtx;
	std::vector<std::shared_ptr<node>> connectinos;
};