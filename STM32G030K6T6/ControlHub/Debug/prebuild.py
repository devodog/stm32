'''
Prebuild script to update build number and version for mcu software
Lines to change.
Line15: #define BUILD 1
Line16: #define MAJOR_VERSION 0
Line17: #define MINOR_VERSION 1
Line18: // The BUILD_DATE_AND_TIME will be updated for each build by a pre-compile session
Line19: #define BUILD_DATE_AND_TIME "31.1.2022 16:35:12"


'''
import datetime

x = datetime.datetime.now()


f = open('appver.h', 'r')
count = 0
line=[]
lineList=[]
space = " "
while True:
    count += 1
    lineBuffer = f.readline()
    if not lineBuffer:
        break 
    lineList.append(lineBuffer)

    if count == 15:
        line = lineList[14].split()
        buildNo = int(line[2])+1
        lineList[14] = line[0]+space+line[1]+space+str(buildNo)+"\n"
    
    if count == 19:
        line = lineList[18].split()
        lineList[18] = line[0]+space+line[1]+space+'"'+str(x)+'"'+"\n"

f.close()
#for s in lineList:
#    print(s,end="")

f = open('appver.h', 'w')
for s in lineList:
    f.write(s)
f.close