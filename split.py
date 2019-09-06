with open("nodup.txt") as f:
    lines = f.readlines()
    print(len(lines))
    i = 0
    file1 = open("d11.txt", "w+")
    file2 = open("d22.txt", "w+")
    file3 = open("d33.txt", "w+")
    file4 = open("d44.txt", "w+")
    file5 = open("d55.txt", "w+")
    file6 = open("d66.txt", "w+")
    file7 = open("d77.txt", "w+")
    file8 = open("d88.txt", "w+")

    for i in range(0, 58396):
        file1.write(lines[i])

    for i in range(58397, 116793):
        file2.write(lines[i])

    for i in range(116794, 175190):
        file3.write(lines[i])

    for i in range(175191, 233587):
        file4.write(lines[i])

    for i in range(233588, 291984):
        file5.write(lines[i])

    for i in range(291985, 348381):
        file6.write(lines[i])

    for i in range(348382, 406778):
        file7.write(lines[i])

    for i in range(406779, 467166):
        file8.write(lines[i])
