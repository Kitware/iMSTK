# This script takes input mkdocs.yml file and generates a navigation
# This exists because auto sorted subdirectories are not possible so one
# either has to manually write the nav or have mkdoc generate the whole
# thing alphabetically sorted
import sys
import os

if len(sys.argv) < 4:
    raise Exception("Too few program arguments, usage: " + sys.argv[0] + " <input docs directory> <input yml file> <output yml file>")
if len(sys.argv) > 4:
    raise Exception("Too many program arguments.")

inputDocDir = sys.argv[1]
inputYmlFile = sys.argv[2]
outputYmlFile = sys.argv[3]

# Check inputs
if not os.path.isdir(inputDocDir):
    raise Exception("Provided input directory <" + inputDocDir + "> does not exists or is not a directory!")
if not os.path.exists(inputYmlFile):
    raise Exception("Provided input yml file <" + inputYmlFile + "> does not exists!")

print("Generating Yml!")
print("inputDocDir: ", inputDocDir)
print("inputYmlFile: ", inputYmlFile)
print("outputYmlFile: ", outputYmlFile)

isSameFile = (inputYmlFile == outputYmlFile)
if isSameFile:
    outputYmlFile = outputYmlFile.replace(".yml", "") + "_output.yml"


# Read the input file and write to out
fileOut = open(outputYmlFile, "wt") # Write
fileIn = open(inputYmlFile, mode="rt")

# Find the line that contains the about the examples page
for line in fileIn:
    loc = line.find("About the Examples")
    if loc == -1:
        fileOut.write(line)
    else:
        fileOut.write(line)
        # For every file (which should be an example md file) in the examples directory
        exampleDirPath = inputDocDir + "/Examples/"
        for x in os.walk(exampleDirPath):
            for filename in x[2]:
                print("filename: ", filename)
                fileNameNoExt = filename.replace(".md", "")
                fileOut.write("        - " + fileNameNoExt + ": 'Examples/" + filename + "'\n")
            break

if isSameFile:
    os.rename(outputYmlFile, inputYmlFile)