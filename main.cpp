#include "btree.h"
#include <iostream>
#include <string>
#include <cstdint>
using namespace std;
static void usage(const string& programname) 
{
    cerr << "Usage:\n";
    cerr << "  " << programname << " create <index_file>\n";
    cerr << "  " << programname << " insert <index_file> <key> <value>\n";
    cerr << "  " << programname << " search <index_file> <key>\n";
    cerr << "  " << programname << " load <index_file> <csv_file>\n";
    cerr << "  " << programname << " print <index_file>\n";
    cerr << "  " << programname << " extract <index_file> <output_csv>\n";
}
static bool parse_uint64(const string& s, uint64_t& value) 
{
    try 
    {
        size_t pos = 0;
        unsigned long long parsed = stoull(s, &pos);
        if (pos != s.size()) 
        {
            return false;
        }

        value = static_cast<uint64_t>(parsed);
        return true;
    } 
    catch (...) 
    {
        return false;
    }
}
int main(int argc, char* argv[]) 
{
    if (argc < 3) 
    {
        usage(argv[0]);
        return 1;
    }
    string command = argv[1];
    string path = argv[2];
    if (command == "create") 
    {
        if (argc != 3) 
        {
            usage(argv[0]);
            return 1;
        }
        return cmdcreate(path);
    }
    else if (command == "insert") 
    {
        if (argc != 5) 
        {
            usage(argv[0]);
            return 1;
        }
        uint64_t key;
        uint64_t value;
        if (!parse_uint64(argv[3], key)) 
        {
            cerr << "Error: invalid key.\n";
            return 1;
        }
        if (!parse_uint64(argv[4], value)) 
        {
            cerr << "Error: invalid value.\n";
            return 1;
        }

        return cmdinsert(path, key, value);
    }
    else if (command == "search") 
    {
        if (argc != 4) 
        {
            usage(argv[0]);
            return 1;
        }

        uint64_t key;

        if (!parse_uint64(argv[3], key)) 
        {
            cerr << "Error: invalid key.\n";
            return 1;
        }
        return cmdsearch(path, key);
    }
    else if (command == "load") 
    {
        if (argc != 4) 
        {
            usage(argv[0]);
            return 1;
        }

        string csvpath = argv[3];

        return cmdload(path, csvpath);
    }
    else if (command == "print") 
    {
        if (argc != 3) 
        {
            usage(argv[0]);
            return 1;
        }

        return cmdprint(path);
    }
    else if (command == "extract") 
    {
        if (argc != 4) 
        {
            usage(argv[0]);
            return 1;
        }
        string out_path = argv[3];
        return cmdextract(path, out_path);
    }
    else 
    {
        cerr << "Error: unknown command '" << command << "'.\n";
        usage(argv[0]);
        return 1;
    }
}