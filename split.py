with open("final1.txt") as f:
    lines = f.readlines()
    print(len(lines))
    i = 0
    file1 = open("d1.txt", "w+")
    file2 = open("d2.txt", "w+")
    file3 = open("d3.txt", "w+")
    file4 = open("d4.txt", "w+")
    file5 = open("d5.txt", "w+")
    file6 = open("d6.txt", "w+")
    file7 = open("d7.txt", "w+")
    file8 = open("d8.txt", "w+")

    for i in range(0, 291633):
        file1.write(lines[i])

    for i in range(291634, 583267):
        file2.write(lines[i])

    for i in range(583268, 874901):
        file3.write(lines[i])

    for i in range(874902, 1166535):
        file4.write(lines[i])

    for i in range(1166536, 1458169):
        file5.write(lines[i])

    for i in range(1458170, 1749803):
        file6.write(lines[i])

    for i in range(1749804, 2041437):
        file7.write(lines[i])

    for i in range(2041438, 2333066):
        file8.write(lines[i])
    print(291633)
    print(583267-291634)
    print(874901 - 583268)
    print(2333066-2041438)
