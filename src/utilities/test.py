#! /usr/bin/python 
# python script for testing named entity recognition
#
# Copyright (C) 2008  Diego Molla-Aliod <diego@ics.mq.edu.au>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
##############################################################################

"python script for testing named entity recognition"

from __future__ import division
import os
import sys
import re

class NamedEntity:
    """A class to store details of a named entity"""
    def __init__(self,token,NeType,leftOffset=-1,rightOffset=-1,prob=1.0):
        self.token = token
        self.leftOffset = leftOffset
        self.rightOffset = rightOffset
        self.neType = NeType
        self.prob = prob
    def getToken(self):
        return self.token
    def getLeftOffset(self):
        return self.leftOffset
    def getRightOffset(self):
        return self.rightOffset
    def getNeType(self):
        return self.neType
    def getProb(self):
        return self.prob
    def matches(self,otherNE):
        return self.token == otherNE.getToken() \
        and self.neType == otherNE.getNeType() \
        and self.leftOffset == otherNE.getLeftOffset() \
        and self.rightOffset == otherNE.getRightOffset() 
        
    def __str__(self):
        return "".join(["Offset: ",str(self.leftOffset),"-",str(self.rightOffset),\
                       "; Word: ",self.token,"; Entity Type: ",self.neType,\
                       "; Probability: ",str(self.prob)])

def getText(filename):
    """Returns the text of a file"""
    f = open(filename)
    text = f.read()
    f.close()
    return text

def matchEntities(marked, unmarked,foundList,notfoundList):
    """Fills the lists foundList and notFoundList"""
    for entity in marked:
        found = False
        for match in unmarked:
            found = entity.matches(match)
            if found:
                foundEntity = match
                break
        if not found:
            notfoundList.append(entity)
        else:
            if foundEntity.getProb() >= 0:
                foundList.append(foundEntity)
            else:
                foundList.append(entity)

def getDates(text):
    """Returns a list of date NEs in the text given"""
    monthex = r'(?:january|jan|february|feb|march|mar|april|apr|may|june|jun|july|jul|august|aug|september|sep|october|oct|november|nov|december|dec)'
    dayex = r'(?:(?:[1-3]?(?:0th|1st|2nd|3rd|[4-9]th)|11th|12th|13th)|(?:3[01]|[0-2]{0,1}\d)|(?:(?:(?:twenty\s|thirty\s)?(?:first|second|third|fourth|fifth|sixth|seventh|eighth|ninth))|tenth|eleventh|twelfth|(?:thir|four|fif|six|seven|eigh|nine)teenth)|twentieth|thirtieth)' #works so far
    yearex = r'(?:\d{4})'
    wdayex = r'(?:mon|monday|tues|tuesday|wed|wednesday|thurs|thur|thursday|fri|friday|sat|saturday|sun|sunday)'
    spacex = r"\s*"
    spommex = r",?\s*"
    joinex = "|"
    datex1 = "".join(["(?:",dayex,spacex,monthex,spommex,yearex,")"])
    datex2 = "".join(["(?:",monthex,spacex,dayex,spommex,yearex,")"])
    datex3 = "".join(["(?:",monthex,spommex,yearex,")"])
    datex4 = "".join(["(?:",dayex,spacex,monthex,")"])
    datex5 = "".join(["(?:",monthex,spacex,dayex,")"])
    datex6 = "".join(["(?:",wdayex,spommex,dayex,spacex,monthex,")"])
    datex7 = "".join(["(?:",wdayex,spommex,dayex,spacex,monthex,spommex,yearex,")"])
    datex8 = "".join(["(?:",wdayex,spommex,monthex,spacex,dayex,spommex,yearex,")"])
    datex = re.compile("".join(["(?i)(?:",datex1,joinex,datex2,joinex,datex3,joinex,datex4,joinex,datex5,joinex,datex6,joinex,datex7,joinex,datex8,")"]))
    out = re.finditer(datex,text)
    NEs = []
    for match in out:
        pre = removeTagsBefore(text,match.start())
        temp = NamedEntity(match.group(0),"DATE",len(pre)+1,len(pre)+len(match.group(0)),-1.0)
        NEs.append(temp)
    return NEs

def extractMarked(text):
    """Extracts the marked-up named entities from some text"""
    regex = r'(?i)<(?:(?:ENAMEX|TIMEX|NUMEX)).+?</(?:(?:ENAMEX|TIMEX|NUMEX))>'
    regex = re.compile(regex)
    res = re.finditer(regex,text)
    ents = []
    for match in res:
        EXType = entityEX(match.group(0))
        eType = entityType(match.group(0))
        eText = entityText(match.group(0))
        pre = removeTagsBefore(text,match.start())
        NE = NamedEntity(eText,eType,len(pre),len(pre)+len(eText),-2.0)
        ents.append(NE)
    return ents

def removeTagsBefore(text,index):
    s = ""
    for i in range(0,index):
        s += text[i]
    o = re.sub(r"<.+?>","",s)
    return o

def entityEX(tag):
    """Extracts the 'ex' type of named entity from a tag"""
    regex = r'(?i)(?:<)(\w+?MEX)'
    regex = re.compile(regex)
    res = re.findall(regex,tag)
    t = ""
    if len(res) > 0:
        t = "".join([res[0]])
    else:
        t = None
    return t

def entityType(tag):
    """Extracts the type of named entity from a tag"""
    # regex = r'(?i)(?:TYPE=\")(.+?)"'
    regex = r'(?i)(<\s*[^/>]+>)'
    regex = re.compile(regex)
    res = re.findall(regex,tag)
    t = ""
    if len(res) > 0:
        t = "".join([res[0]])
    else:
        t = None
    return t

def entityText(tag):
    """Extracts the text of named entity from a tag"""
    regex = r'(?i)(?:>)(.+?)<'
    regex = re.compile(regex)
    res = re.findall(regex,tag)
    t = ""
    if len(res) > 0:
        t = "".join([res[0]])
    else:
        t = None
    return t

def printEnts(entList):
    print "Found " + str(len(entList)) + " entities:"
    return
# The following code will be ignored
    print "First 10 entities:"
    for ent in entList[0:10]:
#        print ent
        print str(ent)

def read_C_output(text, threshold=0):
    """Gets the text of a file outputed from C++ NER and converts to list of NEs"""
    regex = r'(?i)(?:Offset: )(?P<leftOffset>\d+)-(?P<rightOffset>\d+);\s*Word:\s*(?P<token>.+?);\s*Entity Type:\s*(?P<type>.+);\s*Probability:\s*(?P<prob>[0-9.]+)'
    regex = re.compile(regex)
    res = re.findall(regex,text)
    out = []
    for match in res:
        # print "matching with prob", match[4], "and threshold", threshold 
        if float(match[4]) < float(threshold):
            continue
        leftOffset = int(match[0])
        rightOffset = int(match[1])
        token = match[2]
        netype = match[3]
        prob = match[4]
        temp = NamedEntity(token,netype,leftOffset,rightOffset,prob)
        # print str(temp)
        out.append(temp)
    return out
    
def processDirectory(directory):
    """Processes files in a directory"""
    flist = os.listdir(directory)
    for fname in flist:
        try:
            name = "".join([directory,os.sep,fname])
            text = getText(name)
            dateList = getDates(text)   #get a list of date NEs
            markedList = extractMarked(text)
            foundList = []
            notfoundList = []
            unmarkedFound = []
            matchEntities(markedList, dateList,foundList,notfoundList)
            matchEntities(dateList,markedList,[],unmarkedFound)
            print "\n" + name + "\nMARKED ENTITIES"
            printEnts(markedList)
            print "\nUNMARKED ENTITIES"
            printEnts(dateList)
            print "\nCOMMON ENTITIES"
            printEnts(foundList)
            print "\nENTITIES NOT FOUND"
            printEnts(notfoundList)
            print "\nUNMARKED ENTITES FOUND"
            printEnts(unmarkedFound)
            print "\n"
            outputFile = "".join([directory,os.sep,"results",os.sep,fname])
            print outputFile
            op = open(outputFile,'w')
            for i in dateList:
                op.write(str(i)+'\n')
            op.close()
        except IOError:
            pass

def calcScore(annot,results,threshold):
    """Given annotated corpus directory and results directory, obtain recall and precision"""
    flist = os.listdir(annot)
    print flist
    possibleCount = 0
    foundCount = 0
    correctCount = 0
    unmarkedCount = 0
    types = []
    for fname in flist:
        annotName = "".join([annot,os.sep,fname])
        resultName = "".join([results,os.sep,fname])
        print annotName
        print resultName
        try:
            markedText = getText(annotName)
            # print markedText
            # print "..."
            annotList = extractMarked(markedText)
            # print "---"
            # print annotList
            for e in annotList:
                if e.getNeType() not in types:
                    types.append(e.getNeType())
            resultText = getText(resultName)
            resultsList = read_C_output(resultText,threshold)
            foundList = []
            notfoundList = []
            unmarkedFound = []
            matchEntities(annotList, resultsList,foundList,notfoundList)
            matchEntities(resultsList,annotList,[],unmarkedFound)
            print "\n" + fname + "\nMARKED ENTITIES"
            printEnts(annotList)
            print "\nUNMARKED ENTITIES"
            printEnts(resultsList)
            print "\nCOMMON ENTITIES"
            printEnts(foundList)
            print "\nENTITIES NOT FOUND"
            printEnts(notfoundList)
            print "\nUNMARKED ENTITES FOUND"
            printEnts(unmarkedFound)
            try:
                recall = len(foundList)/len(annotList)
            except ZeroDivisionError:
                recall = 0.0
            try:
                precision = len(foundList)/len(resultsList)
            except ZeroDivisionError:
                precision = 0.0
            print "Recall: " + str(round(recall*100,2)) + "%"
            print "Precision: " + str(round(precision*100,2)) + "%"
            print "\n"
            possibleCount += len(annotList)
            foundCount += len(resultsList)
            correctCount += len(foundList)
            unmarkedCount += len(unmarkedFound)
        except IOError:
            print "There was an IO error on file", fname
            pass
    try:
        Trecall = correctCount/possibleCount
    except ZeroDivisionError:
        Trecall = 0.0
    try:
        Tprecision = correctCount/foundCount
    except ZeroDivisionError:
        Tprecision = 0.0
    try:
        fmeasure = (2*Trecall*Tprecision)/(Trecall+Tprecision)
    except ZeroDivisionError:
        fmeasure = 0.0
    print "\nTOTAL STATISTICS"
    print "Total Recall: " + str(round(Trecall*100,2))+ "%"
    print "Total Precision: " + str(round(Tprecision*100,2)) + "%"
    print "F-Measure: " + str(round(fmeasure*100,2)) + "%"
    print types

if __name__ == "__main__":
#    os.system('CLS')
    args = sys.argv
    args.pop(0)
    print len(args)
#    try:
    if args[0].lower() == '-c':
        if len(args) < 4:
            threshold = 0
        else:
            threshold = args[3]
        calcScore(args[1],args[2],threshold)
    else:
        if args[0].lower() == '-p':
            for i in range(1,len(args)):
                processDirectory(args[i])
        else:
            print args
            print """Improper paramaters supplied.
 Must be either:
     For Checking results:
     -c corpusDir resultsDir
     For processing:
     -p directory directory ..."""
