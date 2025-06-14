import sys

n = int(sys.argv[1])
file_name = sys.argv[2]

with open(file_name,"r") as fd:
    content = fd.read().split("\n")
    content = content[:-1]
    for l in content:
        values = l.split("is ")[1]
        values = values.replace(" ","")
        values = values[:-1]
        values_int = []
        for i in values.split(","):
            values_int.append(int(i))
        for i,v in enumerate(values_int):
            print(f"{v} ",end="")
            if (i+1)%n == 0:
                print("")
        print("")