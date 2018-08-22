import pymysql

def check(acc,password):
    db = pymysql.connect("192.168.125.134","root","33979954aaa","education")
    cursor = db.cursor()
    sql = "select * from user where acc = '%s'"%(acc)
    try:
        cursor.execute(sql)
        results = cursor.fetchall()
        for row in results:
            acc_ = row[0]
            pas_ = row[1]
        if (acc_ == acc)&(pas_ == password):
            return True
    except:
        return False