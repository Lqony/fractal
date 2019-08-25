#pragma once

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include "types.h"

constexpr size_t PART_OUT_VALID = 289127755402084;

struct part_out
{
	part_out() : valid(PART_OUT_VALID) {}
	uint64_t valid;
	uint16_t part;
	blob_type blob;
	
	template<class Archive>
	void serialize(Archive &ar, const unsigned int file_version)
	{
		ar & valid;
		ar & part;
		ar & blob;
	}
};

template <typename T>
inline void serialize_to_blob(const T& obj, blob_type& res)
{
	boost::iostreams::back_insert_device<blob_type> inserter(res);
	boost::iostreams::stream<boost::iostreams::back_insert_device<blob_type> > s(inserter);
	boost::archive::binary_oarchive oa(s);
	oa << obj;
	s.flush();
}

template <typename T>
inline void deserialize_from_blob(const blob_type& res, T& obj)
{
	boost::iostreams::basic_array_source<char> device(res.data(), res.size());
	boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
	boost::archive::binary_iarchive ia(s);
	ia >> obj;
}

inline void split_into(const blob_type& blob, uint32_t amount, std::vector<part_out>& res)
{
	uint32_t done = 0;
	uint32_t part_size = blob.size() / amount;
	
	for(int i = 0; i < amount; i++)
	{
		if(i < amount-1)
			part_size = blob.size() - done;
		
		res.emplace_back();
		res.back().part = i+1;
		res.back().blob += blob.substr(done, part_size);
		
		done += part_size;
	}
}

template <typename T>
inline void reconstruct(const std::vector<part_out>& in, T& out)
{
	uint32_t current = 1;
	blob_type blob("");
	
	while(current <= in.size())
	{
		bool speed_up = false;
		
		for(const part_out& po : in)
		{
			//std::cout << po.part << " / " << current << " / " << in.size() << std::endl;
			if(po.part == current)
			{
				blob += po.blob;
				current++;
				if(speed_up || current > in.size())
					break;
			}
			else if(po.part == current+1)
				speed_up = true;
		}
	}
	
	deserialize_from_blob(blob, out);
}