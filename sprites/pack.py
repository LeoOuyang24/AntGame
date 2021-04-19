import cv2 as cv
import numpy as np
import sys
import functools as tools

def main(filename,newName):
    img = cv.imread(filename, cv.IMREAD_UNCHANGED)
    height = int(len(img))
    width = int(len(img[0]))
    copy = []
    for i in range(height): #eliminates blank space above images
        for j in range(width):
            if (img[i][j][3] > 255/2):
                copy.append(list(img[i]))
                break
    i = 0
    prevCol = False;
    maxBuf = 0
    curBuf = 0
    blankColumn = lambda x, y : x and y[i][3] == 0
    while i < len(copy[0]):
        if not tools.reduce(blankColumn, copy, True): #not whole column is blank
            prevCol = True
            curBuf += 1
        else:
            if prevCol:
                if curBuf > maxBuf:
                    maxBuf = curBuf
                curBuf = 0
            prevCol = False
        i += 1
    if curBuf > maxBuf:
        maxBuf = curBuf
    curBuf = 0
    i = 0
    prevCol = False
    while i < len(copy[0]):
        if not tools.reduce(blankColumn, copy, True): #not whole column is blank
            prevCol = True
            curBuf += 1
        else:
            if prevCol:
                if (curBuf < maxBuf):
                    for j in range(maxBuf - curBuf):
                        for row in copy:
                            row.insert(i - curBuf,[0,0,0,0])
                    i += maxBuf - curBuf
                elif (curBuf > maxBuf):
                    for j in range(curBuf - maxBuf):
                        for row in copy:
                            #row[i-curBuf + j] = [0,0,255,255]
                            row.pop(i - curBuf)
                    i -= (curBuf - maxBuf)
                curBuf = 0
            curBuf += 1
            prevCol = False
        i += 1
    if prevCol:
        if (curBuf < maxBuf):
            for j in range(maxBuf - curBuf):
                for row in copy:
                    row.insert(i - curBuf,[0,0,0,0])
            i += maxBuf - curBuf
        elif (curBuf > maxBuf):
            for j in range(curBuf - maxBuf):
                for row in copy:
                    row.pop(i - curBuf)    
    else:
        for j in range(curBuf):
            for row in copy:
                #row[i-curBuf + j] = [0,128,128,255]
                row.pop(i - curBuf)    
    cv.imwrite(newName,np.array(copy))
if __name__ == "__main__":
    if (len(sys.argv) < 2):
        print("usage: python trim.py <image path> <new file name optional>")
    else:
        if (len(sys.argv) < 3): #if they didn't provide a new file name, use the old name
            sys.argv.append(sys.argv[1])
        main(sys.argv[1],sys.argv[2])   