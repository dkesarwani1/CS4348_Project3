#include "btree.h"

#include "byteorder.h"

#include <cstring>

#include <cstdio>

#include <iostream>

#include <fstream>

#include <sstream>

#include <vector>

#include <functional>

#include <sys/stat.h>
using namespace std;
// Header I/O

bool readheader(FILE* f, Header& h) 
{
    if (fseek(f, 0, SEEK_SET) != 0) 
        return false;
    if (fread(h.magic, 1, 8, f) != 8) 
        return false;
    if (!readuint64be(f, h.root_id))
        return false;
    if (!readuint64be(f, h.nextblockid)) 
        return false;
    return true;
}

bool writeheader(FILE* f, const Header& h) 
{

    if (fseek(f, 0, SEEK_SET) != 0) 
        return false;

    if (fwrite(h.magic, 1, 8, f) != 8) 
        return false;

    if (!writeuint64be(f, h.root_id)) 
        return false;

    if (!writeuint64be(f, h.nextblockid)) 
        return false;

    uint8_t pad[blocksize - 24] = {0};

    if (fwrite(pad, 1, sizeof(pad), f) != sizeof(pad)) 
        return false;

    return true;

}
uint64_t allocateblock(Header& h) 
{ 
    return h.nextblockid++; 
}

// helpers

static bool file_exists(const string& path) 
{

    struct stat st;

    return stat(path.c_str(), &st) == 0;

}

bool isleaf(const Node& n) 
{
    for (int i = 0; i < maxchildren; i++)

        if (n.children[i] != 0) return false;
    return true;
}
static long block_offset(uint64_t block_id) 
{
    return static_cast<long>(block_id) * blocksize;
}
// Open an existing index file, validate its magic, and read the header.
static FILE* open_index(const string& path, const char* mode, Header& h) 
{
    FILE* f = fopen(path.c_str(), mode);
    if (!f) 
    {

        cerr << "Error: could not open '" << path << "'.\n";

        return nullptr;

    }
    if (!readheader(f, h)) 
    {
        cerr << "Error: could not read header from '" << path << "'.\n";
        fclose(f);
        return nullptr;
    }
    if (memcmp(h.magic, MAGIC, 8) != 0) 
    {
        cerr << "Error: '" << path << "' is not valid index file.\n";
        fclose(f);
        return nullptr;
    }
    return f;
}
static void init_empty_node(Node& n, uint64_t block_id, uint64_t parentid) 
{
    n.blockid = block_id;
    n.parentid = parentid;
    n.numkeys = 0;
    for (int i = 0; i < maxkeys; i++)    
    { 
        n.keys[i] = 0; n.values[i] = 0; 
    }

    for (int i = 0; i < maxchildren; i++) n.children[i] = 0;

}
// Caller writes node back to disk.
static void leaf_insert(Node& n, uint64_t key, uint64_t value) 
{
    int i = static_cast<int>(n.numkeys) - 1;
    while (i >= 0 && key < n.keys[i]) 
    {
        n.keys[i + 1]   = n.keys[i];
        n.values[i + 1] = n.values[i];
        i--;
    }
    n.keys[i + 1]   = key;
    n.values[i + 1] = value;
    n.numkeys++;
}

// Node I/O

bool writenode(FILE* f, const Node& n) 
{

    if (fseek(f, block_offset(n.blockid), SEEK_SET) != 0) return false;

    if (!writeuint64be(f, n.blockid))   return false;

    if (!writeuint64be(f, n.parentid))  return false;

    if (!writeuint64be(f, n.numkeys))   return false;

    for (int i = 0; i < maxkeys; i++)

        if (!writeuint64be(f, n.keys[i])) return false;

    for (int i = 0; i < maxkeys; i++)

        if (!writeuint64be(f, n.values[i])) return false;

    for (int i = 0; i < maxchildren; i++)

        if (!writeuint64be(f, n.children[i])) return false;

    uint8_t pad[blocksize - 488] = {0};

    if (fwrite(pad, 1, sizeof(pad), f) != sizeof(pad)) return false;

    return true;

}

bool readnode(FILE* f, uint64_t block_id, Node& n) 
{

    if (fseek(f, block_offset(block_id), SEEK_SET) != 0) 
        return false;

    if (!readuint64be(f, n.blockid))  
        return false;

    if (!readuint64be(f, n.parentid)) 
        return false;

    if (!readuint64be(f, n.numkeys))  
        return false;

    for (int i = 0; i < maxkeys; i++)
    {   
        if (!readuint64be(f, n.keys[i])) return false;
    }
    for (int i = 0; i < maxkeys; i++)
    {
        if (!readuint64be(f, n.values[i])) 
            return false;
    }
    for (int i = 0; i < maxchildren; i++)
    {
        if (!readuint64be(f, n.children[i])) 
            return false;
    }
    return true;
}

// ---------- step 4 helpers ----------

static bool key_exists(FILE* f, uint64_t cur, uint64_t key) 
{
    while (cur != 0) 
    {
        Node n;

        if (!readnode(f, cur, n)) 
            return false;

        uint64_t i = 0;

        while (i < n.numkeys && key > n.keys[i]) i++;

        if (i < n.numkeys && key == n.keys[i]) 
            return true;

        if (isleaf(n)) 
            return false;

        cur = n.children[i];
    }

    return false;
}

static bool split_child(FILE* f, Header& h, Node& parent, int child_index) 
{
    Node fullchild;

    if (!readnode(f, parent.children[child_index], fullchild))
        return false;

    bool wasleaf = isleaf(fullchild);

    uint64_t new_id = allocateblock(h);

    Node newchild;

    init_empty_node(newchild, new_id, parent.blockid);

    newchild.numkeys = mindegreee - 1;

    for (int j = 0; j < mindegreee - 1; j++) 
    {
        newchild.keys[j] = fullchild.keys[j + mindegreee];

        newchild.values[j] = fullchild.values[j + mindegreee];

        fullchild.keys[j + mindegreee] = 0;

        fullchild.values[j + mindegreee] = 0;
    }

    if (!wasleaf) 
    {
        for (int j = 0; j < mindegreee; j++) 
        {
            newchild.children[j] = fullchild.children[j + mindegreee];

            fullchild.children[j + mindegreee] = 0;
        }
    }

    uint64_t middlekey = fullchild.keys[mindegreee - 1];

    uint64_t middlevalue = fullchild.values[mindegreee - 1];

    fullchild.keys[mindegreee - 1] = 0;

    fullchild.values[mindegreee - 1] = 0;

    fullchild.numkeys = mindegreee - 1;

    for (int j = static_cast<int>(parent.numkeys); j >= child_index + 1; j--) 
    {
        parent.children[j + 1] = parent.children[j];
    }

    parent.children[child_index + 1] = newchild.blockid;

    for (int j = static_cast<int>(parent.numkeys) - 1; j >= child_index; j--) 
    {
        parent.keys[j + 1] = parent.keys[j];

        parent.values[j + 1] = parent.values[j];
    }

    parent.keys[child_index] = middlekey;

    parent.values[child_index] = middlevalue;

    parent.numkeys++;

    if (!writenode(f, fullchild))
        return false;

    if (!writenode(f, newchild))
        return false;

    if (!writenode(f, parent))
        return false;

    if (!wasleaf) 
    {
        for (int j = 0; j < mindegreee; j++) 
        {
            if (newchild.children[j] != 0) 
            {
                Node grandchild;

                if (!readnode(f, newchild.children[j], grandchild))
                    return false;

                grandchild.parentid = newchild.blockid;

                if (!writenode(f, grandchild))
                    return false;
            }
        }
    }

    return writeheader(f, h);
}

// create 

int cmdcreate(const string& path) 
{
    if (file_exists(path)) 
    {
        cerr << "Error: file '" << path << "' already exists.\n";
        return 1;
    }
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) 
    {
        cerr << "Error: could not create '" << path << "'.\n";
        return 1;
    }

    Header h;

    memcpy(h.magic, MAGIC, 8);

    h.root_id = 0;

    h.nextblockid = 1;

    bool ok = writeheader(f, h);

    fclose(f);

    return ok ? 0 : 1;

}

// insert

int cmdinsert(const string& path, uint64_t key, uint64_t value) {

    Header h;

    FILE* f = open_index(path, "r+b", h);

    if (!f) return 1;

    if (h.root_id != 0 && key_exists(f, h.root_id, key)) 
    {
        cerr << "Error: key " << key << " already exists.\n";

        fclose(f);

        return 1;
    }

    if (h.root_id == 0) 
    {

        uint64_t new_id = allocateblock(h);

        Node root;

        init_empty_node(root, new_id, 0);

        root.keys[0]   = key;

        root.values[0] = value;

        root.numkeys  = 1;

        h.root_id = new_id;

        bool ok = writenode(f, root) && writeheader(f, h);

        fclose(f);

        return ok ? 0 : 1;

    }
    Node root;
    if (!readnode(f, h.root_id, root)) { fclose(f); return 1; }

    if (root.numkeys == maxkeys) 
    {
        uint64_t oldrootid = root.blockid;

        uint64_t newrootid = allocateblock(h);

        root.parentid = newrootid;

        if (!writenode(f, root)) 
        {
            fclose(f);

            return 1;
        }

        Node newroot;

        init_empty_node(newroot, newrootid, 0);

        newroot.children[0] = oldrootid;

        h.root_id = newrootid;

        if (!writenode(f, newroot)) 
        {
            fclose(f);

            return 1;
        }
        if (!split_child(f, h, newroot, 0)) 
        {
            fclose(f);

            return 1;
        }
        root = newroot;
    }
    Node current = root;

    while (true) 
    {
        if (isleaf(current)) 
        {
            leaf_insert(current, key, value);

            bool ok = writenode(f, current) && writeheader(f, h);

            fclose(f);

            return ok ? 0 : 1;
        }

        int i = static_cast<int>(current.numkeys) - 1;

        while (i >= 0 && key < current.keys[i]) 
        {
            i--;
        }

        i++;

        Node child;

        if (!readnode(f, current.children[i], child)) 
        {
            fclose(f);

            return 1;
        }

        if (child.numkeys == maxkeys) 
        {
            if (!split_child(f, h, current, i)) 
            {
                fclose(f);

                return 1;
            }

            if (key > current.keys[i]) 
            {
                i++;
            }

            if (!readnode(f, current.children[i], child)) 
            {
                fclose(f);

                return 1;
            }
        }
        current = child;
    }
}

// search

int cmdsearch(const string& path, uint64_t key) 
{
    Header h;
    FILE* f = open_index(path, "rb", h);
    if (!f) return 1;
    if (h.root_id == 0) 
    {
        cerr << "Key " << key << " not found.\n";
        fclose(f);
        return 1;
    }
    uint64_t cur = h.root_id;
    while (cur != 0) 
    {
        Node n;
        if (!readnode(f, cur, n)) { fclose(f); return 1; }
        uint64_t i = 0;
        while (i < n.numkeys && key > n.keys[i]) i++;

        if (i < n.numkeys && key == n.keys[i]) 
        {

            cout << n.keys[i] << "," << n.values[i] << "\n";

            fclose(f);

            return 0;

        }
        if (isleaf(n)) 
            break;
        cur = n.children[i];
    }
    cerr << "Key " << key << " not found.\n";
    fclose(f);
    return 1;
}
static bool traverse_in_order(FILE* f, Header& h, function<void(uint64_t, uint64_t)> visit) 
{
    if (h.root_id == 0)
        return true;

    struct Frame 
    {
        uint64_t blockid;
        int nextidx;
    };
    vector<Frame> stack;

    stack.push_back({h.root_id, 0});

    while (!stack.empty()) 
    {
        Frame& top = stack.back();

        Node n;

        if (!readnode(f, top.blockid, n))
            return false;

        if (top.nextidx > static_cast<int>(2 * n.numkeys)) 
        {
            stack.pop_back();

            continue;
        }

        int step = top.nextidx;

        top.nextidx++;

        if (step % 2 == 0) 
        {
            int childindex = step / 2;

            if (childindex <= static_cast<int>(n.numkeys) && n.children[childindex] != 0) 
            {
                stack.push_back({n.children[childindex], 0});
            }
        } 
        else 
        {
            int keyindex = step / 2;

            visit(n.keys[keyindex], n.values[keyindex]);
        }
    }

    return true;
}
int cmdload (const string& path, const string& csv_path) 
{
    ifstream in(csv_path);

    if (!in) 
    {
        cerr << "Error: could not open '" << csv_path << "'.\n";

        return 1;
    }

    string line;

    while (getline(in, line)) 
    {
        if (line.empty())
            continue;

        stringstream ss(line);

        string keystr;

        string valuestr;

        if (!getline(ss, keystr, ',') || !getline(ss, valuestr)) 
        {
            cerr << "Error: bad CSV line: " << line << "\n";

            return 1;
        }

        uint64_t key = stoull(keystr);

        uint64_t value = stoull(valuestr);

        if (cmdinsert(path, key, value) != 0)
            return 1;
    }

    return 0;
}

int cmdprint (const string& path) 
{
    Header h;

    FILE* f = open_index(path, "rb", h);

    if (!f) return 1;

    bool ok = traverse_in_order(f, h, [](uint64_t key, uint64_t value) 
    {
        cout << key << "," << value << "\n";
    });

    fclose(f);

    return ok ? 0 : 1;
}

int cmdextract(const string& path, const string& out_path) 
{
    if (file_exists(out_path)) 
    {
        cerr << "Error: output file '" << out_path << "' already exists.\n";

        return 1;
    }
    Header h;
    FILE* f = open_index(path, "rb", h);
    if (!f) return 1;
    FILE* out = fopen(out_path.c_str(), "w");
    if (!out) 
    {
        cerr << "Error: could not create '" << out_path << "'.\n";

        fclose(f);

        return 1;
    }

    bool ok = traverse_in_order(f, h, [&](uint64_t key, uint64_t value) 
    {
        fprintf(out, "%llu,%llu\n",(unsigned long long)key,(unsigned long long)value);
    });

    fclose(out);

    fclose(f);

    return ok ? 0 : 1;

}
