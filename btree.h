#ifndef BTREE_H

#define BTREE_H

#include <cstdint>

#include <cstdio>

#include <string>

constexpr int blocksize   = 512;

constexpr int mindegreee   = 10;                       // t

constexpr int maxkeys     = 2 * mindegreee - 1;       // 19

constexpr int maxchildren = 2 * mindegreee;           // 20

constexpr const char* MAGIC = "4348PRJ3";              // 8 ASCII bytes

struct Header 
{
    char     magic[8];
    uint64_t root_id;
    uint64_t next_block_id;
};

struct Node {

    uint64_t block_id;

    uint64_t parent_id;

    uint64_t num_keys;

    uint64_t keys[maxkeys];

    uint64_t values[maxkeys];

    uint64_t children[maxchildren];

};
// On-disk operations
bool readheader(FILE* f, Header& h);
bool writeheader(FILE* f, const Header& h);
bool readnode  (FILE* f, uint64_t block_id, Node& n);
bool writenode (FILE* f, const Node& n);

// True if every children pointer is 0 (i.e. node has no descendants).

bool isleaf(const Node& n);

// Allocates a new block id by bumping header.next_block_id.

// Caller must write the header back to disk afterward.

uint64_t allocateblock(Header& h);

// Commands (one per CLI subcommand)

int cmdcreate (const std::string& path);

int cmdinsert (const std::string& path, uint64_t key, uint64_t value);

int cmdsearch (const std::string& path, uint64_t key);

int cmdload   (const std::string& path, const std::string& csv_path);

int cmdprint  (const std::string& path);

int cmdextract(const std::string& path, const std::string& out_path);
#endif
