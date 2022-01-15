#include "libchunkedhashtable.h"
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <fstream>
#include <cstring>

ChunkedHashTable::ChunkedHashTable(std::filesystem::path path, unsigned int chunk_size):
file_path{path}, chunk_size{chunk_size}, file_size{std::filesystem::file_size(path)} 
{
	chunk_count = static_cast<unsigned int>(std::ceil(static_cast<double>(file_size)/chunk_size));
	hash_table = new unsigned char[SHA256_DIGEST_LENGTH*chunk_count];
	create_hash_table();
}

ChunkedHashTable::ChunkedHashTable(std::string path, unsigned int chunk_size):ChunkedHashTable::ChunkedHashTable(std::filesystem::path(path), chunk_size){}
ChunkedHashTable::~ChunkedHashTable()
{
	delete[] hash_table;
}
void ChunkedHashTable::create_hash_table()
{
	std::ifstream file(file_path, std::ios::in | std::ios::binary);
	if(file.is_open()){
		char buffer[chunk_size];
		unsigned int i{0};
		while(file.good() && !file.eof()){
			file.read(buffer, chunk_size);
			sha256(buffer, file.gcount(), hash_table+(SHA256_DIGEST_LENGTH*i++));
		}
		/*for(auto i{0};i<chunk_count; ++i){
			print_hash(hash_table+(i*SHA256_DIGEST_LENGTH));
			std::cout << '\n';
		}*/
		file.close();
	}
}

std::vector<int> ChunkedHashTable::check(std::filesystem::path path)
{
	std::vector<int> corrupt_list;
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if(file.is_open()){
		unsigned char buffer[SHA256_DIGEST_LENGTH];
		unsigned int i{0};
		while(file.good() && !file.eof()){
			file.read(reinterpret_cast<char *>(buffer), SHA256_DIGEST_LENGTH);
            if(!file.gcount()) break;
			if(std::memcmp(buffer, hash_table+(SHA256_DIGEST_LENGTH*i), SHA256_DIGEST_LENGTH))
				corrupt_list.push_back(i);
			i++;
		}
		/*for(auto i{0};i<chunk_count; ++i){
			print_hash(hash_table+(i*SHA256_DIGEST_LENGTH));
			std::cout << '\n';
		}*/
		file.close();
	}
	return corrupt_list;
}

void ChunkedHashTable::save(std::filesystem::path path){
	std::ofstream file(path, std::ios::out|std::ios::binary);
	if(file.is_open())
		for(auto i{0};i<chunk_count; ++i)
			file.write(reinterpret_cast<const char *>(hash_table+(i*SHA256_DIGEST_LENGTH)), SHA256_DIGEST_LENGTH);
	file.close();
}

void print_hash(const unsigned char *hash)
{
	std::stringstream ss;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
	std::cout << ss.str();
}

void sha256(const char *data, unsigned int size, unsigned char *buffer)
{
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, data, size);
	SHA256_Final(buffer, &sha256);
}


