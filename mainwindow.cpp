#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QPushButton>
#include <QTextStream>
#include <QRegularExpression>
#include <QSaveFile>
#include <QDebug>
#include <iostream>
/**
 * Costruttore della classe MainWindow
 */
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow){
    currentFileName="";
    ui->setupUi(this);
    font_size=5;
    setFixedSize(this->width(), this->height());
    init_menubar();
    ui->textarea->setReadOnly(true);
    ui->textarea->zoomIn(font_size);
    ui->analyzeButton->setEnabled(false);
    ui->saveFile->setEnabled(false);
    chartView=nullptr;
    escape_chars << "\a"<<"\n"<<"\v"<<"\t"<< "\b"<<""<<"\f"<<"\r"<<"\0";

    //Action
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->analyzeButton, &QPushButton::clicked, this, &MainWindow::showAnalysisTable);
    connect(ui->saveFile, &QPushButton::clicked, this, &MainWindow::saveAnalysisOnCSVFile);
    connect(ui->analyzeButton, &QPushButton::clicked, this, &MainWindow::showAnalysisChart);
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);

}
/**
 * Funzione che mostra i dati calcolati nella tabella
 */
void MainWindow::showAnalysisTable() const{
    ui->table->setItem(0,0, new QTableWidgetItem(QString::number(countCharacters())));
    ui->table->setItem(0,1, new QTableWidgetItem(QString::number(countOnlyCharacters())));
    ui->table->setItem(0,2, new QTableWidgetItem(QString::number(countWords())));
    ui->table->setItem(0,3, new QTableWidgetItem(QString::number(countPhrase())));
    ui->table->setItem(0,4, new QTableWidgetItem(QString::number(countParagraphs())));
}

/**
 * Funzione che mostra il bar chart associato al testo del file.
 * Il bottone associato non si attiva finchè non viene premuto
 * il bottone "Analyze text"
 */
void MainWindow::showAnalysisChart(){
    if(chartView==nullptr){
        QList<unsigned int> values={countCharacters(),countOnlyCharacters(), countWords(), countPhrase(), countParagraphs()};
        QBarSet *set0 = new QBarSet("Caratteri (punteggiatura e spazi)");
        QBarSet *set1 = new QBarSet("Caratteri (spazi esclusi)");
        QBarSet *set2 = new QBarSet("Parole");
        QBarSet *set3 = new QBarSet("Frasi");
        QBarSet *set4 = new QBarSet("Paragrafi");
        *set0 << values[0];
        *set1 << values[1];
        *set2 << values[2];
        *set3 << values[3];
        *set4 << values[4];

        unsigned int max = *std::max_element(values.begin(), values.end());
        QBarSeries *series = new QBarSeries();
        series->append(set0);
        series->append(set1);
        series->append(set2);
        series->append(set3);
        series->append(set4);

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Text Analysis");
        chart->setAnimationOptions(QChart::AllAnimations);

        QStringList categories;
        categories<<"";
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0, max);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        QPalette pal = qApp->palette();
        pal.setColor(QPalette:: Window, QRgb(0xffffff));
        pal.setColor(QPalette:: WindowText, QRgb(0x404040));
        qApp->setPalette(pal);

        ui->chartLayout->addWidget(chartView);
        show();
        ui->saveFile->setEnabled(true);

    }
}
/**
 * Funzione che crea e salva il file csv associato al testo
 * nella cartella "file_csv" all'interno della
 * directory del progetto.
 * Il file viene salvato solo dopo che si clicca il bottone associato, che a sua volta
 * si sblocca se viene cliccato il bottone "Analyze text".
 */
void MainWindow::saveAnalysisOnCSVFile(){
    QString filename=QFileInfo(currentFileName).baseName();

    QFile data("./file_csv/"+filename+".csv");
    if(data.open(QFile::WriteOnly |QFile::Truncate)){
        QTextStream output(&data);
        output << "Caratteri (inclusa punteggiatura e spazi): \t"<<countCharacters()<<"\n";
        output << "Caratteri (spazi esclusi): \t"<<countOnlyCharacters()<<"\n";
        output << "Parole: \t"<<countWords()<<"\n";
        output << "Frasi: \t"<<countPhrase()<<"\n";
        output << "Paragrafi: \t"<<countParagraphs();
        data.close();
        ui->lineEdit->setText(QFileInfo(data.fileName()).absoluteFilePath().toLatin1());
    }
}
/**
 * Funzione che conta il numero dei paragrafi
 * Se il testo contiene solo spazi o caratteri di escape il valore
 * ritornato sarà 0
 */
unsigned int MainWindow::countParagraphs() const{
    unsigned int paragraph=0;
    int blanks=0, escape=0;

    if(!ui->textarea->document()->isEmpty()){
        QString text=ui->textarea->document()->toPlainText();
        for(int i=0; i<text.size(); ++i){
            if(text[i].unicode()==32)
                blanks++;
            if(escape_chars.contains(QString(text[i])))
                escape++;
        }
        text.remove("\t");
        if(blanks!=text.size() && escape!=text.size())
            paragraph = text.split(QRegExp("(\\n)+"), QString::SkipEmptyParts).count();
    }
    return paragraph;
}


/**
 * Funzione che conta il numero delle parole
 */
unsigned int MainWindow::countWords() const{
    unsigned int words=0;
    QString text = ui->textarea->toPlainText();
    if(!ui->textarea->document()->isEmpty())
        words = text.split(QRegExp("(\\s|\\n|\\r)+"), QString::SkipEmptyParts).count();

    return words;
}
/**
 * Funzione che conta il numero dei caratteri (spazi esclusi)
 * I caratteri presi in considerazione sono tutti quelli che
 * hanno un valore unicode minore o uguali di 65535 (escluso il 32)
 */
unsigned int MainWindow::countOnlyCharacters() const{
    unsigned int tot=0;
    unsigned int max=65535;
    QString text=ui->textarea->document()->toPlainText();
    if(!ui->textarea->document()->isEmpty()){
        for(int i=0; i<text.size(); ++i)
            if(!escape_chars.contains(QString(text[i])) && text[i].unicode()<=max && text[i].unicode()!=32)
                tot++;

    }
    return tot;
}

/**
 * Funzione che conta il numero dei caratteri.
 * I caratteri presi in considerazione sono tutti quelli che
 * hanno un valore unicode minore o uguali di 65535
 */
unsigned int MainWindow::countCharacters() const{
    unsigned int tot=0;
    unsigned int max=65535;
    if(!ui->textarea->document()->isEmpty()){
        QString text=ui->textarea->document()->toPlainText();
        for(int i=0; i<text.size(); ++i)
            if(!escape_chars.contains(QString(text[i])) && text[i].unicode()<=max) //spazio 32 incluso
                tot++;
    }
    return tot;
}
/**
 * Funzione che conta il numero delle frasi
 * Se il testo contiene solo spazi o caratteri di escape il valore
 * ritornato sarà 0
 */
unsigned int MainWindow::countPhrase() const{
    unsigned int phrases=0;
    int blanks=0, escape=0;

    if(!ui->textarea->document()->isEmpty()){
        QString text=ui->textarea->toPlainText();
        text.remove("\n");
        text.remove(" ");
        for(int i=0; i<text.size(); ++i){
            if(text[i].unicode()==32)
                blanks++;
            if(escape_chars.contains(QString(text[i])))
                escape++;
        }

        if(blanks!=text.size() && escape!=text.size())
            phrases = text.split(QRegExp("(\\.|\\?|\\!)+"), QString::SkipEmptyParts).count();


    }
    return phrases;
}


/**
 * Funzione che crea il menu bar con due azione:
 * apertura del file e chiusura dell'applicazione
 */
void MainWindow::init_menubar(){
    fileMenu=menuBar()->addMenu("File");
    openAction=new QAction(QIcon(),"Open");
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("Open a existing file");
    fileMenu->addAction(openAction);
    fileMenu->addSeparator();
    exitAction=new QAction(QIcon(), "Exit");
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip("Exit program");
    fileMenu->addAction(exitAction);
}

/**
 * Funzione che apre la dialog per caricare il contenuto del file nel QPlainTextEdit.
 * Se precedentemente è stato analizzato un file allora cancella il grafico e
 * svuota la tabella
 */
void MainWindow::openFile(){
    currentFileName=QFileDialog::getOpenFileName(this);
    if(!currentFileName.isEmpty()){
        QFile file(currentFileName);// QFile file(currentFileName);

        if(!file.open(QFile::ReadOnly | QFile::Text))
            QMessageBox::warning(this,"Warning", "Error during file loading");

        QTextStream in(&file);
        in.setCodec("UTF-8");
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        ui->textarea->document()->setPlainText(in.readAll());
        QGuiApplication::restoreOverrideCursor();
        ui->analyzeButton->setEnabled(true);

        delete chartView;
        chartView=nullptr;
        ui->saveFile->setEnabled(false);
        ui->table->setItem(0,0, new QTableWidgetItem(""));
        ui->table->setItem(0,1, new QTableWidgetItem(""));
        ui->table->setItem(0,2, new QTableWidgetItem(""));
        ui->table->setItem(0,3, new QTableWidgetItem(""));
        ui->table->setItem(0,4, new QTableWidgetItem(""));
        ui->lineEdit->setText("");
    }
}



/**
 * Distruttore della classe
 */
MainWindow::~MainWindow(){
    delete ui;
    delete fileMenu;
    delete openAction;
    delete exitAction;
    delete chartView;
}

