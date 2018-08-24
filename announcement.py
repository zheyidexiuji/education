import pymysql
def getannouncemnet(time):
    db = pymysql.connect("192.168.125.134","root","33979954aaa","education")
    cursor = db.cursor()
    sql = "select announ from announcement where time = '%s'"%(time)
    text = ''
    try:
        cursor.execute(sql)
        results = cursor.fetchall()
        for result in results:
            text = text + str(result[0])
            text = text+'\n'
            text = text+'                                  '
            text = text+'\n'
        return text
    except:
        return '请重试'



