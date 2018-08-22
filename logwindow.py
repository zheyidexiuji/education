import sys
from PyQt5.QtWidgets import QApplication,QWidget,QPushButton,QDesktopWidget,QLabel,QMessageBox
import check
from PyQt5.Qt import QLineEdit
from PyQt5.QtCore import QCoreApplication
import MainW as M
class Logwindow(QWidget):
    def __init__(self):
        super().__init__()
        self.InitGUI()
    def InitGUI(self):
        self.lab1 = QLabel('账号',self)
        self.lab1.move(23,30)
        self.lab2 = QLabel('密码',self)
        self.lab2.move(23,80)
        self.text1 = QLineEdit(self)
        self.text1.move(50,30)
        self.text1.resize(150,15)
        self.text2 = QLineEdit(self)
        self.text2.move(50,80)
        self.text2.resize(150,15)
        self.text2.setEchoMode(QLineEdit.Password)
        self.btn = QPushButton('登陆',self)
        self.btn.move(30,150)
        self.btn.clicked.connect(lambda:self.check())
        self.btn2 = QPushButton('退出',self)
        self.btn2.move(150,150)
        self.btn2.clicked.connect(lambda :QCoreApplication.instance().quit())
        self.setWindowTitle('登录')
        self.resize(250,250)
        self.center()
        self.show()
    def center(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())
    def check(self):
        acc = self.text1.text()
        pas = self.text2.text()
        #if(check.check(acc,pas)):
        if(True):
            MW = M.MainW(acc)
            self.close()
            pass
        else:
            QMessageBox.critical(self,'login','账号密码错误')
def main():
    app = QApplication(sys.argv)
    Log = Logwindow()
    sys.exit(app.exec_())
if __name__ == '__main__':
    main()
