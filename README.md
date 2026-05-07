# CS4348_Project3

## How to compile
g++ -std=c++17 -g -Wall -o project3 main.cpp byteorder.cpp btree.cpp

### Commands format
./project3 create <index_file>
./project3 insert <index_file> <key> <value>
./project3 search <index_file> <key>
./project3 load <index_file> <csv_file>
./project3 print <index_file>
./project3 extract <index_file> <output_csv>

### Create a new index file
rm -f test.idx
./project3 create test.idx

### Example: Insert values
./project3 insert test.idx 10 100
./project3 insert test.idx 5 50
./project3 insert test.idx 20 200

### Example: Search for key
./project3 search test.idx 10
Output: 10,100

### Example: Print all records
./project3 print test.idx
Output:
5,50
10,100
20,200
### Load from CSV File
Create a CSV File and then Load
cat > data.csv <<EOF
30,3000
40,4000
50,5000
EOF

./project3 load test.idx data.csv
./project3 print test.idx
Expected Output:
5,50
10,100
20,200
30,3000
40,4000
50,5000

### Extract to CSV
rm -f output.csv
./project3 extract test.idx output.csv
cat output.csv

Expected output:
5,50
10,100
20,200
30,3000
40,4000
50,5000
