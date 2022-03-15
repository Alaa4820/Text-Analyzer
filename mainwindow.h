#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QStatusBar>
#include <QToolBar>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QtCharts/QChartView>

#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void openFile();
    void showAnalysisTable() const;
    void showAnalysisChart();

private:
    Ui::MainWindow *ui;
    QMenu *fileMenu;
    QAction *openAction;
    QAction *exitAction;
    QString currentFileName;
    QChartView *chartView;
    QStringList escape_chars;
    int font_size;

    void init_menubar();
    void saveAnalysisOnCSVFile();

    unsigned int countOnlyCharacters() const;
    unsigned int countCharacters() const;
    unsigned int countWords() const;
    unsigned int countPhrase() const;
    unsigned int countParagraphs() const;

};
#endif // MAINWINDOW_H
