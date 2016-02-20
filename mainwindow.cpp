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
#include <QWebView>
#include <QWebInspector>
#include <QWebFrame>

#include "mainwindow.h"
#include "scriptproxy.h"

MainWindow::MainWindow()
{
	setWindowIcon(QIcon::fromTheme("accessories-text-editor"));

	webView = new QWebView;
	setCentralWidget(webView);

	webInspectorDock = new QDockWidget(tr("Inspector"));
	webInspectorDock->setObjectName("webInspectorDock");
	webInspector = new QWebInspector;
	webInspector->setPage(webView->page());
	webView->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	webInspectorDock->setWidget(webInspector);
	addDockWidget(Qt::BottomDockWidgetArea, webInspectorDock);
	webInspectorDock->hide();

	// Attach this to JavaScript and keep it attached when page reloads
	scriptProxy = new ScriptProxy(this);
	connect(scriptProxy, SIGNAL(isModifiedChanged(bool)), this, SLOT(documentWasModified()));
	attachToWebPage();
	connect(webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(attachToWebPage()));

	// Create GUI
	createActions();
	createToolBars();
	createMenus();
	createStatusBar();

	readSettings();

	setCurrentFile("");
	setUnifiedTitleAndToolBarOnMac(true);

	webView->load(QUrl("qrc:/example_editor.html"));
}


/**
 * Attach this object to web page, so it is available from JavaScript.
 */
void MainWindow::attachToWebPage()
{
	scriptProxy->setObjectName("ScriptProxy");
	webView->page()->mainFrame()->addToJavaScriptWindowObject(QString("WebWrap"), scriptProxy);
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
		scriptProxy->clear();
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
	QMessageBox::about(this, tr("About WebWrapEditor"),
			tr("WebWrapEditor " GIT_VERSION "\n"
				"\n"
				"Universal editor wrapper, which wraps HTML5-based "
				"editor with a desktop window."));
}


void MainWindow::documentWasModified()
{
	setWindowModified(scriptProxy->isModified());
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

	undoAct = new QAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this);
	undoAct->setShortcuts(QKeySequence::Undo);
	undoAct->setStatusTip(tr("Undo recent action"));
	//connect(undoAct, SIGNAL(triggered()), webView, SLOT(undo()));

	redoAct = new QAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this);
	redoAct->setShortcuts(QKeySequence::Redo);
	redoAct->setStatusTip(tr("Redo recent action"));
	//connect(redoAct, SIGNAL(triggered()), webView, SLOT(redo()));

	cutAct = new QAction(QIcon::fromTheme("edit-cut"), tr("Cu&t"), this);
	cutAct->setShortcuts(QKeySequence::Cut);
	cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
	//connect(cutAct, SIGNAL(triggered()), webView, SLOT(cut()));

	copyAct = new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
	copyAct->setShortcuts(QKeySequence::Copy);
	copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
	//connect(copyAct, SIGNAL(triggered()), webView, SLOT(copy()));

	pasteAct = new QAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this);
	pasteAct->setShortcuts(QKeySequence::Paste);
	pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
	//connect(pasteAct, SIGNAL(triggered()), webView, SLOT(paste()));

	aboutAct = new QAction(QIcon::fromTheme("help-about"), tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	debugConsoleAct = webInspectorDock->toggleViewAction();
	debugConsoleAct->setIcon(QIcon::fromTheme("utilities-terminal"));
	debugConsoleAct->setCheckable(true);
	debugConsoleAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
	debugConsoleAct->setStatusTip(tr("Show web inspector"));

	/*
	cutAct->setEnabled(false);
	connect(webView, SIGNAL(copyAvailable(bool)),
			cutAct, SLOT(setEnabled(bool)));

	copyAct->setEnabled(false);
	connect(webView, SIGNAL(copyAvailable(bool)),
			copyAct, SLOT(setEnabled(bool)));

	undoAct->setEnabled(false);
	connect(webView, SIGNAL(undoAvailable(bool)),
			undoAct, SLOT(setEnabled(bool)));

	redoAct->setEnabled(false);
	connect(webView, SIGNAL(redoAvailable(bool)),
			redoAct, SLOT(setEnabled(bool)));
	*/
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
	editMenu->addAction(undoAct);
	editMenu->addAction(redoAct);
	editMenu->addSeparator();
	editMenu->addAction(cutAct);
	editMenu->addAction(copyAct);
	editMenu->addAction(pasteAct);

	viewMenu = createPopupMenu();
	viewMenu->setTitle(tr("&View"));
	menuBar()->addMenu(viewMenu);

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutQtAct);
}


void MainWindow::createToolBars()
{
	fileToolBar = addToolBar(tr("File toolbar"));
	fileToolBar->setObjectName("fileToolBar");
	fileToolBar->addAction(newAct);
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);

	editToolBar = addToolBar(tr("Edit toolbar"));
	editToolBar->setObjectName("editToolBar");
	editToolBar->addAction(undoAct);
	editToolBar->addAction(redoAct);
	editToolBar->addSeparator();
	editToolBar->addAction(cutAct);
	editToolBar->addAction(copyAct);
	editToolBar->addAction(pasteAct);

	debugToolBar = addToolBar(tr("Debug toolbar"));
	debugToolBar->setObjectName("debugToolBar");
	debugToolBar->addAction(debugConsoleAct);
	debugToolBar->hide();
}


void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}


void MainWindow::readSettings()
{
	QSettings settings("WebWrapEditor", "WebWrapEditor");
	settings.beginGroup("mainWindow");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	settings.endGroup();
}


void MainWindow::writeSettings()
{
	QSettings settings("WebWrapEditor", "WebWrapEditor");
	settings.beginGroup("mainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.endGroup();
}


bool MainWindow::maybeSave()
{
	/*
	if (webView->document()->isModified()) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, QCoreApplication::applicationName(),
				tr("The document has been modified.\n"
					"Do you want to save your changes?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save)
			return save();
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	*/
	return true;
}


bool MainWindow::loadFile(const QString &fileName)
{
	QFile file(fileName);

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::warning(this, QCoreApplication::applicationName(),
				tr("Cannot read file %1:\n%2.")
				.arg(fileName)
				.arg(file.errorString()));
		return false;
	}

#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
#endif

	// Create new script proxy and populate it
	QTextStream in(&file);
	scriptProxy->clear();
	scriptProxy->setFileName(fileName);
	scriptProxy->setEditorData(in.readAll());
	scriptProxy->setModified(false);

#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif
	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File loaded"), 2000);

	return true;
}


bool MainWindow::saveFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, QCoreApplication::applicationName(),
				tr("Cannot write file %1:\n%2.")
				.arg(fileName)
				.arg(file.errorString()));
		return false;
	}

	QTextStream out(&file);
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
	out << scriptProxy->editorData();
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
	scriptProxy->setModified(false);
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


