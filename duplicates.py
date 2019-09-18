with open("dictleet3.txt") as f:
    nodup = open("final1.txt", "w+")
    lines_seen = set()
    lines = f.readlines()
    for l in lines:
        if l not in lines_seen:
            nodup.write(l)
            lines_seen.add(l)
    nodup.close()
