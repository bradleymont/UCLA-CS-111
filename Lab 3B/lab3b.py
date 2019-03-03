#!/usr/bin/python

# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

import sys #for command-line argument
import csv #for parsing through the given csv file

superBlock = None

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

class Group:
    def __init__(self, row):
        #note: row[0] is "GROUP"
        self.groupNum = int(row[1])
        self.numBlocks = int(row[2])
        self.numInodes = int(row[3])
        self.numFreeBlocks = int(row[4])
        self.numFreeInodes = int(row[5])

groups = []

freeBlocks = []

freeInodes = []

inodes = []

class Inode:
    def __init__(self, row):
        self.inodeNum = int(row[1])
        self.fileType = row[2]
        self.mode = int(row[3])
        self.owner = int(row[4])
        self.group = int(row[5])
        self.linkCount = int(row[6])
        self.ctime = row[7]
        self.mtime = row[8]
        self.atime = row[9]
        self.fileSize = int(row[10])
        self.numBlocks = int(row[11])
        self.directBlocks = map(int, row[12:24])
        self.singleIndirectBlock = int(row[24])
        self.doubleIndirectBlock = int(row[25])
        self.tripleIndirectBlock = int(row[26])

class Dirent:
    def __init__(self, row):
        self.parentInodeNum = int(row[1])
        self.logicalByteOffset = int(row[2])
        self.referenceInodeNum = int(row[3])
        self.entryLength = int(row[4])
        self.nameLength = int(row[5])
        self.name = row[6]

dirents = []

class Indirect:
    def __init__(self, row):
        self.inodeNum = int(row[1])
        self.levelOfIndirection = int(row[2])
        self.logicalBlockOffset = int(row[3])
        self.indirectBlockNum = int(row[4])
        self.referenceBlockNum = int(row[5])

indirects = []

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
            global groups
            groups.append(Group(row))
        elif entryType == "BFREE":
            global freeBlocks
            freeBlockNum = row[1]
            freeBlocks.append(int(freeBlockNum))
        elif entryType == "IFREE":
            global freeInodes
            freeInodeNum = row[1]
            freeInodes.append(int(freeInodeNum))
        elif entryType == "INODE":
            global inodes
            inodes.append(Inode(row))
        elif entryType == "DIRENT":
            global dirents
            dirents.append(Dirent(row))
        elif entryType == "INDIRECT":
            global indirects
            indirects.append(Indirect(row))
        else:
            sys.stderr.write("Error: Invalid line found in csv file\n")
            exit(1)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.stderr.write("Error: Incorrect argument.\nUsage: ./lab3b fileName.csv\n")
        exit(1)

    parseCSV()
