# CS4348 Project 3 - B-Tree Index

## How to Compile

```bash
g++ -std=c++17 -g -Wall -o project3 main.cpp byteorder.cpp btree.cpp
```

## Command Format

```bash
./project3 create <index_file>
./project3 insert <index_file> <key> <value>
./project3 search <index_file> <key>
./project3 load <index_file> <csv_file>
./project3 print <index_file>
./project3 extract <index_file> <output_csv>
```

## Testing all the Commands

### 1. Create new index file

```bash
rm -f test.idx
./project3 create test.idx
```

### 2. Insert Values

```bash
./project3 insert test.idx 10 100
./project3 insert test.idx 5 50
./project3 insert test.idx 20 200
```

### 3. Search for Key

```bash
./project3 search test.idx 10
```

Expected output:

```text
10,100
```

### 4. Print all records

```bash
./project3 print test.idx
```

Expected output:

```text
5,50
10,100
20,200
```

### 5. Load from CSV File

Create the CSV file:

```bash
cat > data.csv <<EOF
30,3000
40,4000
50,5000
EOF
```

Load the CSV file and print the index:

```bash
./project3 load test.idx data.csv
./project3 print test.idx
```

Expected output:

```text
5,50
10,100
20,200
30,3000
40,4000
50,5000
```

### 6. Extract to CSV File

```bash
rm -f output.csv
./project3 extract test.idx output.csv
cat output.csv
```

Expected output:

```text
5,50
10,100
20,200
30,3000
40,4000
50,5000
```