import pymysql
def getgrade(classname):
    db = pymysql.connect("localhost", "root", "33979954aaa", "education")
    curosr = db.cursor()
    sql = "select * from %s"%(classname)

    curosr.execute(sql)
    results = curosr.fetchall()
    datas =[['id','姓名','语文','数学','英语','物理','化学','生物','总分']]
    try:
        for row in results:
            datas.append(list(row))
        text = ''
        for data in datas:
            if (data[0] != 'id'):
                data[8] = sum(data[2:8])
            for value in data:
                text = text + str(value) + "    "
            text = text + '\n'
        return text
    except:
        return"错误，请重试"
