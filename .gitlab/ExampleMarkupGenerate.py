# This script takes input example directory and generates markup files containing them in the output directory
import sys
import os

if len(sys.argv) < 3:
    raise Exception("Too few program arguments, usage: " + sys.argv[0] + " <input example directory> <output markup directory>")
if len(sys.argv) > 3:
    raise Exception("Too many program arguments.")

rootPath = sys.argv[1]
outputPath = sys.argv[2]

if not os.path.isdir(rootPath):
    raise Exception("Provided input path <" + rootPath + "> does not exists or is not a directory!")
if not os.path.isdir(outputPath):
    raise Exception("Provided output path <" + outputPath + "> does not exists or is not a directory!")

print("Generating Example Markup!")
print("Input Directory: ", rootPath)
print("Output Directory: ", outputPath)

def generateExampleMarkup(dirPath):
    #print("Leaf Directory: ", dirPath)

    exampleName = os.path.basename(dirPath)
    #print("Example Name: ", exampleName)

    # Check for an example descriptor file, we only generated markup docs for
    # files with this md
    descPath = dirPath + "/desc.md"
    #print("Descriptor Path: ", descPath)
    if os.path.exists(descPath):
        print("Generating markup for: ", descPath)

        # Create a markup file for our documentation by copying it
        fileIn = open(descPath, "rt") # Read
        fileOut = open(outputPath + "/" + exampleName + ".md", "wt") # Write

        # Replace all inserts
        for line in fileIn:
            loc = line.find("[cpp_insert]:")
            if loc != -1:
                # Remove spaces and >
                line = line[loc + 13:].replace("<", "").replace(">", "").lstrip()
                #print ("line:", line)

                # Read in and insert the file
                fileOut.write("**" + line + "**\n")
                fileOut.write("```cpp\n")
                exampleFileIn = open(dirPath + "/" + line, mode="r")
                fileOut.write(exampleFileIn.read())
                fileOut.write("\n```\n")
            else:
                fileOut.write(line)

    return

# For every subdirectory (include childs of childs)
for x in os.walk(rootPath):
    #print("Checking: ", x[0])

    # Check if the directory has any child directories
    if len(x[1]) == 0:
        #print("is leaf")
        generateExampleMarkup(x[0])