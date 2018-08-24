from PyQt5.Qt import QMainWindow,QApplication,QAction,qApp,QTextEdit,QToolBar,QHBoxLayout,QFileDialog,QWidget,QMessageBox,QInputDialog
from PyQt5.QtWidgets import QLineEdit
from PyQt5.QtCore import Qt
import File as F
import sys
import upfile
import changepassword as ch
import datetime
import announcement as an
class MainW(QMainWindow):
    def __init__(self,acc):
        super().__init__()
        self.resize(1000,618)
        self.InitGUI()
        self.acc = acc
    def InitGUI(self):
        self.widget = QWidget()
        self.review = QTextEdit()
        grid = QHBoxLayout()
        grid.addWidget(self.review)
        self.creat_tool()
        self.setWindowTitle('Education')
        self.widget.setLayout(grid)
        self.setCentralWidget(self.widget)
        self.statusBar().showMessage('未选择')
        self.show()
    def creat_tool(self):
        btn1 = QAction( '打开文件', self)
        #exitAction.setShortcut('Ctrl+Q')
        btn1.triggered.connect(lambda :self.openfile())
        btn2 = QAction('保存文件',self)
        btn2.triggered.connect(lambda :self.savefile())
        btn3 = QAction('上传文件',self)
        btn3.triggered.connect(lambda :self.upfile())
        btn4 = QAction('成绩',self)
        #btn4.triggered.connect()
        btn5 = QAction('公告',self)
        btn5.triggered.connect(lambda :self.getannouncement())
        btn6 =QAction('更改密码',self)
        btn6.triggered.connect(lambda :self.change())
        btn7 =QAction('选择班级',self)
        btn7.triggered.connect(lambda :self.select())
        self.toolbar=QToolBar()
        self.addToolBar(Qt.LeftToolBarArea,self.toolbar)
        self.toolbar.addAction(btn1)
        self.toolbar.addAction(btn2)
        self.toolbar.addAction(btn3)
        self.toolbar.addAction(btn4)
        self.toolbar.addAction(btn5)
        self.toolbar.addAction(btn6)
        self.toolbar.addAction(btn7)
    def openfile(self):
        fname = QFileDialog.getOpenFileName(self, 'Open file')
        if fname[0]:
            self.review.setText(F.Openfile(fname[0]))
    def savefile(self):
        str = self.review.toPlainText()
        fname = QFileDialog.getSaveFileName(self,'Save file')
        if fname[0]:
            F.savefile(fname[0],str)
    def upfile(self):
        fname = QFileDialog.getOpenFileName(self,"up file")
        if fname[0]:
            information = upfile.up(fname[0])
            QMessageBox.information(self,"上传",information)
    def closeEvent(self, event):
        reply = QMessageBox.question(self, '退出', '你确定要退出吗?', QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if reply == QMessageBox.Yes:
            event.accept()
        else:
            event.ignore()
    def change(self):
        while True:
            newpas, ok = QInputDialog.getText(self, "输入新密码", "输入新密码\n\n请输入:", QLineEdit.Password)
            newpas1, ok = QInputDialog.getText(self, "重复输入", "重复输入\n\n请输入:", QLineEdit.Password)
            if newpas == newpas1:
                bool = ch.change(self.acc,newpas)
                break
                pass
            else:
                QMessageBox.information(self,"错误","密码不同",QMessageBox.Yes)
        if(bool):
            QMessageBox.information(self,'提示','修改成功',QMessageBox.Yes)
            pass
        else:
            QMessageBox.information(self, '提示', '错误，请重试', QMessageBox.Yes)
            pass
    def select(self):
        items = ["1", "2", "3", "4"]
        value, ok = QInputDialog.getItem(self, "选择班级", "请选择班级:", items, 1, True)
        self.statusBar().showMessage(value)
    def getannouncement(self):
        time = datetime.date.today()
        text = an.getannouncemnet(time)
        self.review.setText(text)
def main(acc):
    app1 = QApplication(sys.argv)
    MW = MainW(acc)
    sys.exit(app1.exec_())