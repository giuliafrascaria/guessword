with open("dictionary.txt") as f:
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

    for i in range(0, 32686):
        file1.write(lines[i])

    for i in range(32687, 65374):
        file2.write(lines[i])

    for i in range(65374, 98061):
        file3.write(lines[i])

    for i in range(98062, 130748):
        file4.write(lines[i])

    for i in range(130749, 163435):
        file5.write(lines[i])

    for i in range(163436, 196122):
        file6.write(lines[i])

    for i in range(196123, 228809):
        file7.write(lines[i])

    for i in range(228810, 261490):
        file8.write(lines[i])
