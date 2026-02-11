#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Set application metadata
  QApplication::setApplicationName("SerialFlow");
  QApplication::setApplicationVersion("1.0");
  QApplication::setOrganizationName("SerialFlow");

  // Load stylesheet
  QFile file(":/style.qss");
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    QTextStream stream(&file);
    app.setStyleSheet(stream.readAll());
  }

  MainWindow window;
  window.show();

  return app.exec();
}
