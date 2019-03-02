#!/usr/bin/python

# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

import sys #for command-line argument
import csv #for parsing through the given csv file

# global variables
superBlock = None

# classes for each type of entry in the csv
class SuperBlock:
    def __init__(self, row):
        #note: row[0] is "SUPERBLOCK"
        self.numBlocks = int(row[1])
        self.numInodes = int(row[2])
        self.blockSize = int(row[3])
        self.inodeSize = int(row[4])
        self.blocksPerGroup = int(row[5])
        self.inodesPerGroup = int(row[6])
        self.firstNonResInode = int(row[7])

def parseCSV():
    csvFile = open(sys.argv[1], 'r') #open for reading
    reader = csv.reader(csvFile)
    for row in reader:  #parse each line in the csv
        if len(row) <= 0: #check for empty lines
            sys.stderr.write("Error: Empty line found in csv file\n")
            exit(1)

        entryType = row[0]

        if entryType == "SUPERBLOCK":
            global superBlock #so we can write to the global superBlock inside our function
            superBlock = SuperBlock(row)
        elif entryType == "GROUP":
            print("group")
        elif entryType == "BFREE":
            print("bfree")
        elif entryType == "IFREE":
            print("ifree")
        elif entryType == "INODE":
            print("inode")
        elif entryType == "DIRENT":
            print("dirent")
        elif entryType == "INDIRECT":
            print("indirect")
        else:
            sys.stderr.write("Error: Invalid line found in csv file\n")
            exit(1)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.stderr.write("Error: Incorrect argument.\nUsage: ./lab3b fileName.csv\n")
        exit(1)

    parseCSV()
