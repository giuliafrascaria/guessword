import datetime



with open("top250.txt") as f:
  lines = f.readlines()
  new = open("topanddates.txt","w+")
  for l in lines:
      wlist = l.strip().split('\t')
      print(wlist[3])
      word = wlist[3] + "\n"
      new.write(word)
      up = wlist[3].upper()
      if up != wlist[3]:
          upw = up + "\n"
          new.write(upw)
  start = datetime.datetime.strptime("01-01-1960", "%m-%d-%Y")
  end = datetime.datetime.strptime("12-31-1980", "%m-%d-%Y")
  date_generated = [start + datetime.timedelta(days=x) for x in range(0, (end-start).days)]

  for date in date_generated:
    dateline = date.strftime("%m%d%y") + "\n"
    new.write(dateline)      
  new.close()
