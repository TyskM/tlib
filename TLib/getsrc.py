
import os
import sys

dir = sys.argv[1]

includeQuotes = False
if len(sys.argv) > 2:
    includeQuotes = True

for root, subdirs, files in os.walk(dir):
    for f in files:
        if f.endswith((".c", ".cpp")):
            fixedRoot = root.split(".\\")
            
            if includeQuotes:
                v = f"\"{fixedRoot[-1]}\\{f}\""
            else:
                v = f"{fixedRoot[-1]}\\{f}"

            v = v.replace("\\", "/")
            print(v)