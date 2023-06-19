import os
from parse import parse

def readFile(filePath):
    lines = []
    with open(filePath, "r") as fin:
        lines = fin.readlines()
    lines = [i[:-1] for i in lines]
    res = parse("{}:{}", lines[0])
    ogPath = res[0]
    size = int(res[1])
    # read the readTree
    index = 2
    readTree = []
    writeTree = []
    calls = []
    backups = []
    while(lines[index]!='Write Tree'):
        res = parse("[{}, {}]", lines[index])
        index+=1
        readTree.append((int(res[0]), int(res[1])))
    index +=1
    while(lines[index]!='Calls '):
        res = parse("[{}, {}]", lines[index])
        index+=1
        writeTree.append((int(res[0]), int(res[1])))
    index +=1
    while(lines[index]!='Backups '):
        res = parse("{} {} {} {} {} {}", lines[index])
        calls.append(list(res))
        index+=1
    index +=1
    for i in range(index, len(lines)):
        res = parse("{}:{}", lines[i])
        backups.append((int(res[0]), int(res[1])))
    return ((ogPath, size), readTree, writeTree, calls, backups)

def grabAndFlush(fdToWrite, fdToRead, dataObj):
    # first read from the apt file
    os.lseek(fdToRead, dataObj["start"], os.SEEK_SET)
    data  = os.read(fdToRead, dataObj["end"]-dataObj["start"])
    # flush it to the write File
    os.write(fdToWrite, data)

def createSubset(readTree, backups, ogPath):
    combined = []
    head, tail = os.path.split(ogPath)
    newFilePath = "../AuditLog/{}".format(tail)
    ogFd = os.open(ogPath, os.O_RDONLY)
    if len(backups) != 0:
        backupFD = os.open("../AuditLog/Backups/{}".format(tail),  os.O_RDONLY)

    for obj in readTree:
        #read the data and add it
        os.lseek(ogFd, obj[0], os.SEEK_SET)
        combined.append({
            "start": obj[0],
            "end": obj[1],
            "data": os.read(ogFd, obj[1]-obj[0])
        })
    backupsRev = reversed(backups)
    for obj in backupsRev:
        combined.append({
            "start": obj[0],
            "end": obj[1],
            "data": os.read(backupFD, obj[1]-obj[0])
        })

    os.close(ogFd)
    if len(backups)!=0:
        os.close(backupFD)
    combined.sort(key= lambda x: x["start"])
    return combined
def flushSubset(fileName, subset, size, ogPATH, calls):
    subFD = os.open("../AuditLog/SubsetData/{}.subset".format(fileName), os.O_WRONLY|os.O_CREAT)
    ptrFD = os.open("../AuditLog/SubsetData/{}.pointers".format(fileName), os.O_WRONLY|os.O_CREAT)
    traceFD = os.open("../AuditLog/SubsetData/{}.trace".format(fileName), os.O_WRONLY|os.O_CREAT)
    os.write(ptrFD, bytes("{}\n".format(ogPATH), 'utf-8'))
    os.write(ptrFD, bytes("{}\n".format(size), 'utf-8'))
    fileLoc = 0
    for obj in subset:
        os.write(subFD, obj["data"])
        os.write(ptrFD, bytes("{}:{}:{}\n".format(obj["start"],obj["end"], fileLoc), 'utf-8'))
        fileLoc+=(obj["end"]-obj["start"])
    os.close(subFD)
    os.close(ptrFD)
    for call in calls:
        printItem = ":".join(call)
        os.write(traceFD, bytes("{}\n".format(printItem), 'utf-8'))
    os.close(traceFD)
def parseFiles():
    listFiles = os.listdir("../AuditLog/")
    os.makedirs("../AuditLog/SubsetData",exist_ok=True)
    for file in listFiles:
        if file == 'Backups':
            continue
        ret = readFile("../AuditLog/{}".format(file))
        subset = createSubset(ret[1],ret[-1], ret[0][0])
        flushSubset(file, subset, ret[0][1], ret[0][0], ret[-2])
if __name__ == "__main__":
    parseFiles()
