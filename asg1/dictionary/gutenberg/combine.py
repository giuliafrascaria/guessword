with open("11len.txt") as file1:
    with open("13len.txt") as file2:
        combo = open("combinations.txt", "w+")
        lines1 = file1.readlines()
        lines2 = file2.readlines()
        for l1 in lines1:
            w1 = l1.split()
            for l2 in lines2:
                w2 = l2.split()
                c = w1[0] + w2[0] + "\n"
                combo.write(c)
        combo.close()
