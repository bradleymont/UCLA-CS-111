#!/usr/bin/python

# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

import sys #for command-line argument
import csv #for parsing through the given csv file
from collections import defaultdict #for blockReferences

#global arrays to contain data about each type of csv entry type (besides super block since there is only 1)
exitStatus = 0
superBlock = None
groups = []
freeBlocks = []
freeInodes = []
inodes = []
dirents = []
indirects = []
blockReferences = defaultdict(list) #maps blockNum to corresponding BlockReference object
allocatedInodes = []
unallocatedInodes = []

#classes to represent each type of csv entry type
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
        self.inodeTableSize = self.numInodes * self.inodeSize / self.blockSize + 1
        self.firstLegalBlock = self.inodeTableSize + 4  #4 for the superblock + group descriptor table + block bitmap + inode bitmap

class Group:
    def __init__(self, row):
        #note: row[0] is "GROUP"
        self.groupNum = int(row[1])
        self.numBlocks = int(row[2])
        self.numInodes = int(row[3])
        self.numFreeBlocks = int(row[4])
        self.numFreeInodes = int(row[5])

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

class Indirect:
    def __init__(self, row):
        self.inodeNum = int(row[1])
        self.levelOfIndirection = int(row[2])
        self.logicalBlockOffset = int(row[3])
        self.indirectBlockNum = int(row[4])
        self.referenceBlockNum = int(row[5])

class BlockReference:
    def __init__(self, blockNum, inode, offset, level):
        self.blockNum = blockNum
        self.inode = inode
        self.offset = offset
        self.level = level

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

levelStrings = ["", "INDIRECT ", "DOUBLE INDIRECT ", "TRIPLE INDIRECT "]

def checkBlock(blockNum, inode, offset, level):
    global blockReferences, exitStatus
    if blockNum == 0:
        return
    
    #invalid
    if blockNum < 0 or blockNum > superBlock.numBlocks:
        print("INVALID " + str(levelStrings[level]) +  "BLOCK " + str(blockNum) + " IN INODE " + str(inode) + " AT OFFSET " + str(offset))
        exitStatus = 2
    #reserved
    elif blockNum < superBlock.firstLegalBlock:
        print("RESERVED " + str(levelStrings[level]) +  "BLOCK " + str(blockNum) + " IN INODE " + str(inode) + " AT OFFSET " + str(offset))
        exitStatus = 2
    #valid
    else:
        blockReferences[blockNum].append(BlockReference(blockNum, inode, offset, level))

def auditBlockConsistency():
    global blockReferences, exitStatus
    #direct block pointers for each inode
    for currInode in inodes:
        
        #for symbolic links with length 60 bytes or less, the i-node block pointer fields do not contain block numbers and should not be analyized
        if currInode.fileType == 's' and currInode.fileSize <= 60:
            continue
        
        offset = 0
        for block in currInode.directBlocks:
            checkBlock(block, currInode.inodeNum, offset, 0)
            offset += 1

        checkBlock(currInode.singleIndirectBlock, currInode.inodeNum, 12, 1)
        checkBlock(currInode.doubleIndirectBlock, currInode.inodeNum, 268, 2)
        checkBlock(currInode.tripleIndirectBlock, currInode.inodeNum, 65804, 3)

    #indirect block pointers
    for currIndirect in indirects:
        checkBlock(currIndirect.referenceBlockNum, currIndirect.inodeNum, currIndirect.logicalBlockOffset, currIndirect.levelOfIndirection)

    #examine all valid block references
    for block in range(superBlock.firstLegalBlock, superBlock.numBlocks):
        #If a block is not referenced by any file and is not on the free list
        if block not in blockReferences and block not in freeBlocks:
            print("UNREFERENCED BLOCK " + str(block))
            exitStatus = 2
        #A block that is allocated to some file might also appear on the free list
        elif block in blockReferences and block in freeBlocks:
            print("ALLOCATED BLOCK " + str(block) + " ON FREELIST")
            exitStatus = 2
        #If a legal block is referenced by multiple files (or even multiple times in a single file)
        elif block in blockReferences and len(blockReferences[block]) > 1:
            for currBlockRef in blockReferences[block]:
                print("DUPLICATE " + str(levelStrings[currBlockRef.level]) +  "BLOCK " + str(currBlockRef.blockNum) + " IN INODE " + str(currBlockRef.inode) + " AT OFFSET " + str(currBlockRef.offset))
                exitStatus = 2

def auditInodeAllocation():
    global exitStatus, unallocatedInodes, allocatedInodes
    unallocatedInodes = freeInodes
    for currInode in inodes:
        #allocated inodes
        if currInode.fileType != '0':
            if currInode.inodeNum in freeInodes:
                print("ALLOCATED INODE " + str(currInode.inodeNum) + " ON FREELIST")
                exitStatus = 2
                unallocatedInodes.remove(currInode.inodeNum)
            allocatedInodes.append(currInode)
        #unallocated inodes
        else:
            if currInode.inodeNum not in freeInodes:
                print("UNALLOCATED INODE " + str(currInode.inodeNum) + " NOT ON FREELIST")
                exitStatus = 2
                unallocatedInodes.append(currInode.inodeNum)

        #for all non-reserved inodes
        for currInode in range(superBlock.firstNonResInode, superBlock.numInodes):
            if currInode not in freeInodes:
                allocated = False
                for inode in inodes:
                    if inode.inodeNum == currInode:
                        allocated = True
                if not allocated:
                    print("UNALLOCATED INODE " + str(currInode) + " NOT ON FREELIST")
                    exitStatus = 2
                    unallocatedInodes.append(currInode)

def auditDirectoryConsistency():
    global exitStatus, unallocatedInodes, allocatedInodes
    numInodes = superBlock.numInodes
    
    inodes2numLinks = {} #maps inode numbers to the number of links
    
    for currDirent in dirents:
        if currDirent.referenceInodeNum > numInodes:
            print("DIRECTORY INODE " + str(currDirent.parentInodeNum) + " NAME " + str(currDirent.name) + " INVALID INODE " + str(currDirent.referenceInodeNum))
            exitStatus = 2
        elif currDirent.referenceInodeNum in unallocatedInodes:
            print("DIRECTORY INODE " + str(currDirent.parentInodeNum) + " NAME " + str(currDirent.name) + " UNALLOCATED INODE " + str(currDirent.referenceInodeNum))
            exitStatus = 2
        else:
            newLinkCount = inodes2numLinks.get(currDirent.referenceInodeNum, 0) + 1 #get returns 0 if unable to find referenceInodeNum
            inodes2numLinks[currDirent.referenceInodeNum] = newLinkCount

    for currInode in allocatedInodes:
        if currInode.inodeNum in inodes2numLinks:
            # allocated I-node whose reference count does not match the number of discovered links
            if inodes2numLinks[currInode.inodeNum] != currInode.linkCount:
                print("INODE " + str(currInode.inodeNum) + " HAS " + str(inodes2numLinks[currInode.inodeNum]) + " LINKS BUT LINKCOUNT IS " + str(currInode.linkCount))
                exitStatus = 2
        else:
            if currInode.linkCount != 0:
                print("INODE " + str(currInode.inodeNum) + " HAS 0 LINKS BUT LINKCOUNT IS " + str(currInode.linkCount))
                exitStatus = 2

    inodes2parents = {2: 2} # 2 = root-directory inode
    
    #fill out inodes2parent
    for currDirent in dirents:
        if currDirent.referenceInodeNum <= superBlock.numInodes and currDirent.referenceInodeNum not in unallocatedInodes:
            if currDirent.name != "'.'" and currDirent.name != "'..'":
                inodes2parents[currDirent.referenceInodeNum] = currDirent.parentInodeNum

    for currDirent in dirents:
        if currDirent.name == "'..'":
            if inodes2parents[currDirent.parentInodeNum] != currDirent.referenceInodeNum:
                print("DIRECTORY INODE " + str(currDirent.parentInodeNum) + " NAME '..' LINK TO INODE " + str(currDirent.referenceInodeNum) + " SHOULD BE " + str(inodes2parents[currDirent.parentInodeNum]))
                exitStatus = 2
        elif currDirent.name == "'.'":
            if currDirent.referenceInodeNum != currDirent.parentInodeNum:
                print("DIRECTORY INODE " + str(currDirent.parentInodeNum) + " NAME '.' LINK TO INODE " + str(currDirent.referenceInodeNum) + " SHOULD BE " + str(currDirent.parentInodeNum))
                exitStatus = 2

if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.stderr.write("Error: Incorrect argument.\nUsage: ./lab3b fileName.csv\n")
        exit(1)
    
    parseCSV()
    auditBlockConsistency()
    auditInodeAllocation()
    auditDirectoryConsistency()

    exit(exitStatus)
