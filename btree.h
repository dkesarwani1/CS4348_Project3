#ifndef BTREE_H
#define BTREE_H
#include <cstdint>
#include <cstdio>
#include <string>

constexpr int blocksize   = 512;
constexpr int mindegreee   = 10;                       // t
constexpr int maxkeys     = 2 * mindegreee - 1;       
constexpr int maxchildren = 2 * mindegreee;          
constexpr const char* MAGIC = "4348PRJ3";              // 8 ASCII bytes
struct Header 
{
    char     magic[8];
    uint64_t root_id;
    uint64_t next_block_id;
};

struct Node 
{
    uint64_t blockid;
    uint64_t parentid;
    uint64_t numkeys;
    uint64_t keys[maxkeys];
    uint64_t values[maxkeys];
    uint64_t children[maxchildren];

};
bool readheader(FILE* f, Header& h);
bool writeheader(FILE* f, const Header& h);
bool readnode  (FILE* f, uint64_t block_id, Node& n);
bool writenode (FILE* f, const Node& n);
bool isleaf(const Node& n);

uint64_t allocateblock(Header& h);

int cmdcreate (const std::string& path);
int cmdinsert (const std::string& path, uint64_t key, uint64_t value);
int cmdsearch (const std::string& path, uint64_t key);
int cmdload   (const std::string& path, const std::string& csv_path);
int cmdprint  (const std::string& path);
int cmdextract(const std::string& path, const std::string& out_path);
#endif
