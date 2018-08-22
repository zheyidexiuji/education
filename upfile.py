import re
import xlrd
import pymysql
def up(name):
    L = re.match('(.*)\.(.*)', name)
    if (L.group(2) == "xlsx") or (L.group(2) == 'xls'):
        db = pymysql.connect("localhost","root","33979954aaa","education")
        curror = db.cursor()
        workbook = xlrd.open_workbook(name)
        booksheet = workbook.sheet_by_index(0)
        nrowss = booksheet.nrows
        cla = booksheet.row_values(0)
        claname = cla[2]
        if claname == "数学":
            for i in range(1,nrowss):
                row = booksheet.row_values(i)
                sql = "update 一年级一班 set math = '%s' where id = '%d'" % (row[2], int(row[0]))
                curror.execute(sql)
                db.commit()
            db.close()
            return "上传成功"
        if claname == "语文":
            for i in range(1,nrowss):
                row = booksheet.row_values(i)
                sql = "update 一年级一班 set chines = '%s' where id = '%d'"%(row[2],int(row[0]))
                curror.execute(sql)
                db.commit()
            db.close()
            return "上传成功"
        if claname == "英语":
            for i in range(1,nrowss):
                row = booksheet.row_values(i)
                sql = "update 一年级一班 set english = '%s' where id = '%d'" % (row[2], int(row[0]))
                curror.execute(sql)
                db.commit()
            db.close()
            return "上传成功"
        if claname == "生物":
            for i in range(1,nrowss):
                row = booksheet.row_values(i)
                sql = "update 一年级一班 set bio = '%s' where id = '%d'" % (row[2], int(row[0]))
                curror.execute(sql)
                db.commit()
            db.close()
            return "上传成功"
        if claname == "物理":
            for i in range(1,nrowss):
                row = booksheet.row_values(i)
                sql = "update 一年级一班 set phy = '%s' where id = '%d'" % (row[2], int(row[0]))
                curror.execute(sql)
                db.commit()
            db.close()
            return "上传成功"
        if claname == "化学":
            for i in range(1,nrowss):
                row = booksheet.row_values(i)
                sql = "update 一年级一班 set chem = '%s' where id = '%d'" % (row[2], int(row[0]))
                curror.execute(sql)
                db.commit()
            db.close()
            return "上传成功"
    else:
        return "格式错误"
