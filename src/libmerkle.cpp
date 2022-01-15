#include "libmerkle.h"
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <fstream>

namespace Merkle
{

MerkleHashTree::MerkleHashTree(std::filesystem::path path, unsigned int chunk_size):
file_path{path}, chunk_size{chunk_size}, file_size{std::filesystem::file_size(path)} 
{
	chunk_count = static_cast<unsigned int>(std::ceil(static_cast<double>(file_size)/chunk_size));
	std::cout << path << "\n" << chunk_size << '\n' << file_size << '\n' << chunk_count << '\n' ;
	int depth = static_cast<int>(std::ceil(std::log2(chunk_count+1)));
	tree.reserve(depth);
	tree.end()->reserve(chunk_count);
	MerkleNode t{};
	for(auto i{0}; i<depth; ++i)
		tree.push_back(std::vector<MerkleNode>(1<<i, t));
	std::cout << "Depth: " << tree.size() << '\n';
	for(auto item: tree) std::cout << item.size() << ", ";
	std::cout << '\n';
	for(auto row: tree){
		for(auto item: row)
			std::cout << &item << ' ';
		std::cout  << '\n';
	}
	create_merkle_leaves();
}

MerkleHashTree::MerkleHashTree(std::string path, unsigned int chunk_size):MerkleHashTree::MerkleHashTree(std::filesystem::path(path), chunk_size){}

void MerkleHashTree::create_merkle_leaves()
{
/*	std::ifstream file(file_path, std::ios::in | std::ios::binary);
	if(file.is_open()){
		char buffer[chunk_size];
		unsigned int i{0};
		while(file.good() && !file.eof()){
			file.read(buffer, chunk_size);
			sha256(buffer, file.gcount(), leaves[i++].hash);
		}
		std::cout << chunk_count << ',' <<  i << '\n';
		for(auto i{0};i<chunk_count; i+=2){
			print_hash(leaves[i].hash);
			std::cout << '\n';
			
			print_hash(leaves[i+1].hash);
			std::cout << '\n';

		}
		file.close();
	}*/
}

void MerkleHashTree::create_merkle_roots()
{
	
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
}
