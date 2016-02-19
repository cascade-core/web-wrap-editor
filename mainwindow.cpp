/*
 * Copyright (c) 2015  Josef Kufner <josef@kufner.cz>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <QtWidgets>

#include "mainwindow.h"

MainWindow::MainWindow()
{
	textEdit = new QPlainTextEdit;
	setCentralWidget(textEdit);

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	readSettings();

	connect(textEdit->document(), SIGNAL(contentsChanged()),
			this, SLOT(documentWasModified()));

	setCurrentFile("");
	setUnifiedTitleAndToolBarOnMac(true);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave()) {
		writeSettings();
		event->accept();
	} else {
		event->ignore();
	}
}


void MainWindow::newFile()
{
	if (maybeSave()) {
		textEdit->clear();
		setCurrentFile("");
	}
}


void MainWindow::open()
{
	if (maybeSave()) {
		QString fileName = QFileDialog::getOpenFileName(this);
		if (!fileName.isEmpty())
			loadFile(fileName);
	}
}


bool MainWindow::save()
{
	if (curFile.isEmpty()) {
		return saveAs();
	} else {
		return saveFile(curFile);
	}
}


bool MainWindow::saveAs()
{
	QFileDialog dialog(this);
	dialog.setWindowModality(Qt::WindowModal);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	QStringList files;
	if (dialog.exec())
		files = dialog.selectedFiles();
	else
		return false;

	return saveFile(files.at(0));
}


void MainWindow::about()
{
	QMessageBox::about(this, tr("About Application"),
			tr("The <b>Application</b> example demonstrates how to "
				"write modern GUI applications using Qt, with a menu bar, "
				"toolbars, and a status bar."));
}


void MainWindow::documentWasModified()
{
	setWindowModified(textEdit->document()->isModified());
}


void MainWindow::createActions()
{
	newAct = new QAction(QIcon::fromTheme("document-new"), tr("&New"), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Create a new file"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

	openAct = new QAction(QIcon::fromTheme("document-open"), tr("&Open..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	saveAct = new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
	saveAct->setShortcuts(QKeySequence::Save);
	saveAct->setStatusTip(tr("Save the document to disk"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	saveAsAct = new QAction(tr("Save &As..."), this);
	saveAsAct->setShortcuts(QKeySequence::SaveAs);
	saveAsAct->setStatusTip(tr("Save the document under a new name"));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	cutAct = new QAction(QIcon::fromTheme("edit-cut"), tr("Cu&t"), this);
	cutAct->setShortcuts(QKeySequence::Cut);
	cutAct->setStatusTip(tr("Cut the current selection's contents to the "
				"clipboard"));
	connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));

	copyAct = new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
	copyAct->setShortcuts(QKeySequence::Copy);
	copyAct->setStatusTip(tr("Copy the current selection's contents to the "
				"clipboard"));
	connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));

	pasteAct = new QAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this);
	pasteAct->setShortcuts(QKeySequence::Paste);
	pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
				"selection"));
	connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

	aboutAct = new QAction(QIcon::fromTheme("help-about"), tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	cutAct->setEnabled(false);
	copyAct->setEnabled(false);
	connect(textEdit, SIGNAL(copyAvailable(bool)),
			cutAct, SLOT(setEnabled(bool)));
	connect(textEdit, SIGNAL(copyAvailable(bool)),
			copyAct, SLOT(setEnabled(bool)));
}


void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(cutAct);
	editMenu->addAction(copyAct);
	editMenu->addAction(pasteAct);

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutQtAct);
}


void MainWindow::createToolBars()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(newAct);
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(cutAct);
	editToolBar->addAction(copyAct);
	editToolBar->addAction(pasteAct);
}


void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}


void MainWindow::readSettings()
{
	QSettings settings("QtProject", "Application Example");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(400, 400)).toSize();
	resize(size);
	move(pos);
}


void MainWindow::writeSettings()
{
	QSettings settings("QtProject", "Application Example");
	settings.setValue("pos", pos());
	settings.setValue("size", size());
}


bool MainWindow::maybeSave()
{
	if (textEdit->document()->isModified()) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("Application"),
				tr("The document has been modified.\n"
					"Do you want to save your changes?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save)
			return save();
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	return true;
}


void MainWindow::loadFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Application"),
				tr("Cannot read file %1:\n%2.")
				.arg(fileName)
				.arg(file.errorString()));
		return;
	}

	QTextStream in(&file);
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
	textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File loaded"), 2000);
}


bool MainWindow::saveFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Application"),
				tr("Cannot write file %1:\n%2.")
				.arg(fileName)
				.arg(file.errorString()));
		return false;
	}

	QTextStream out(&file);
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
	out << textEdit->toPlainText();
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File saved"), 2000);
	return true;
}


void MainWindow::setCurrentFile(const QString &fileName)
{
	curFile = fileName;
	textEdit->document()->setModified(false);
	setWindowModified(false);

	QString shownName = curFile;
	if (curFile.isEmpty())
		shownName = "untitled.txt";
	setWindowFilePath(shownName);
}


QString MainWindow::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

