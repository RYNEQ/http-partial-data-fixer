#ifndef LIBCHUNKEDHASHTABLE_H
#define LIBCHUNKEDHASHTABLE_H

#include <filesystem>
#include <string>
#include <cstdint>
#include <vector>
#include <openssl/sha.h>


void sha256(const char *data, unsigned int size,unsigned char *buffer);
void print_hash(const unsigned char *hash);

class ChunkedHashTable{
	std::filesystem::path file_path;
	unsigned int chunk_size;
	unsigned int chunk_count;
	std::uintmax_t file_size;
	unsigned char *hash_table;

	void create_hash_table();

public:
	ChunkedHashTable(std::filesystem::path path, unsigned int chunk_size=4*1024*1024);
	ChunkedHashTable(std::string path, unsigned int chunk_size=4*1024*1024);
	~ChunkedHashTable();
	void save(std::filesystem::path path);
	std::vector<int> check(std::filesystem::path path);
};

#endif
