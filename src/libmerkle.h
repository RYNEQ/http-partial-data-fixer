#ifndef LIBMERKLE_H
#define LIBMERKLE_H

#include <filesystem>
#include <string>
#include <cstdint>
#include <vector>
#include <openssl/sha.h>

namespace Merkle
{
struct _MerkleNode;
using MerkleNode = _MerkleNode; 
struct _MerkleNode{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	MerkleNode *LEFT{nullptr}, *RIGHT{nullptr};
};

void sha256(const char *data, unsigned int size,unsigned char *buffer);
void print_hash(const unsigned char *hash);

class MerkleHashTree{
	std::filesystem::path file_path;
	unsigned int chunk_size;
	unsigned int chunk_count;
	std::uintmax_t file_size;
	std::vector<std::vector<MerkleNode>> tree;

	void create_merkle_leaves();
	void create_merkle_roots();

public:
	MerkleHashTree(std::filesystem::path path, unsigned int chunk_size=4096);
	MerkleHashTree(std::string path, unsigned int chunk_size=4096);

};
}
#endif
