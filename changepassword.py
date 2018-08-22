import pymysql
def change(acc,newpas):
    try:
        db = pymysql.connect("192.168.125.134", "root", "33979954aaa", "education")
        cursor = db.cursor()
        sql = "update user set pass = '%s' where acc = '%s'"%(newpas,acc)
        cursor.execute(sql)
        db.commit()
        db.close()
        return True
    except:
        return False
