# This script removes all non-entity tags from documents
#
# Daniel Smith
import os
import re
import sys

def removeTags(text):
  regex = re.compile(r'</?(?:ROOT.*?|DOC.*?)>')
  return regex.sub("",text)

if __name__ == "__main__":
  # for each argument
  try:
    dirList = os.listdir(sys.argv[1])
    outDir = sys.argv[2]
    for fname in dirList:
      text = ""
      inp = sys.argv[1] + os.sep + fname
      out = outDir + os.sep + fname
      try:
        f = open(inp,'r')
        text = f.read()
        f.close()
        fo = open(out,'w')
        fo.write(removeTags(text))
        fo.close()
      except:
        print "Could not open " + inp
  except IndexError:
    print "Invalid input. Run with inputDir and outputDir."
