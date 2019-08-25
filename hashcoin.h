#include <iostream>

#include "crypto.h"

class hashcoin
{
public:
	hashcoin(coin_type value, key_type ver_key, key_type enc_key) : value(value), ver_key(ver_key), enc_key(enc_key) {}

	size_t get_value() { return value; }
	key_type get_veryfier() { return ver_key; };
	key_type get_encryptor() const { return enc_key; } 
	
protected:
	//Amount this coin is carrying
	coin_type value;

	//Veryfier decrypts signature into hash of message, can not encrypt signature into hash of message
	key_type ver_key;
	
	//Encryptor, encrypts the message so it is only readable for the hashcoin owner (who posses decryptor)
	key_type enc_key;
};

class my_hashcoin : public hashcoin
{
public:
	my_hashcoin(coin_type value, key_type ver_key, key_type enc_key, key_type sign_key, key_type dec_key) : hashcoin(value, ver_key, enc_key), 
	sign_key(sign_key), dec_key(dec_key) {}
	
	key_type get_signer() { return sign_key; };
	key_type get_decryptor() { return dec_key; }
	
private:
	//Signer encrypts hash of message into singature
	key_type sign_key;
	
	//Decryptor, to read encrypted messages
	key_type dec_key;
};

struct transaction
{
	uint64_t value;
	key_type ver_key; 
	key_type enc_key;
	
	template<class Archive>
	void serialize(Archive &ar, const unsigned int file_version)
	{
		ar & value;
		ar & ver_key;
		ar & enc_key;
	}
};