import re
import xlrd
from openpyxl import Workbook
def Openfile(name):
    L = re.match('(.*)\.(.*)',name)
    if (L.group(2) == "xlsx") or (L.group(2) == 'xls'):
        workbook = xlrd.open_workbook(name)
        booksheet = workbook.sheet_by_index(0)
        nrowss = booksheet.nrows
        datas = []
        for i in range(nrowss):
            row = booksheet.row_values(i)
            datas.append(row)
        text = ''
        for data in datas:
            for value in data:
                text = text+str(value)+"  "
            text = text + '\n'
        return text
    else:
        with open(name,'r') as f:
            data = f.read()
        return data
def savefile(name,str):
    L = re.match('(.*)\.(.*)',name)
    if (L.group(2) == "xlsx") or (L.group(2) == 'xls'):
        i=0
        j=0
        str = str.replace("\n","")
        datas = str.split("    ")
        workbook = Workbook()
        booksheet = workbook.active
        for data in datas:
            booksheet.cell(i+1, j+1).value=data
            j=j+1
            if j == 9:
                i=i+1
                j=0
        workbook.save(name)
    else:
        with open(name,'w') as f:
            f.write(str)







