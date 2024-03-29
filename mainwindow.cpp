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
#include <QWebHistory>
#include <QProgressBar>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>

#include "mainwindow.h"
#include "scriptproxy.h"
#include "settingsdialog.h"
#include "plaintextedit.h"

MainWindow::MainWindow()
{
	setWindowIcon(QIcon::fromTheme("accessories-text-editor"));

	// Setup web view
	webView = new QWebView;
	webView->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
	setCentralWidget(webView);

	// Setup inspector & dock
	webInspectorDock = new QDockWidget(tr("Inspector"));
	webInspectorDock->setObjectName("webInspectorDock");
	webInspector = new QWebInspector;
	webInspectorDock->setWidget(webInspector);
	addDockWidget(Qt::BottomDockWidgetArea, webInspectorDock);
	webInspectorDock->hide();

	// Configure Web Page
	QWebPage *webPage = webView->page();
	webInspector->setPage(webPage);
	webPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	webPage->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);

	// Attach this to JavaScript and keep it attached when page reloads
	scriptProxy = new ScriptProxy(this);
	connect(scriptProxy, SIGNAL(isModifiedChanged(bool)), this, SLOT(documentWasModified()));
	attachToWebPage();
	connect(webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(attachToWebPage()));

	// Create buffer view dock
	editorDataDock = new QDockWidget(tr("Editor data"));
	editorDataDock->setObjectName("editorDataDock");
	editorDataView = new PlainTextEdit;
	editorDataView->setReadOnly(true);
	editorDataView->setWordWrapMode(QTextOption::NoWrap);
	editorDataView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	editorDataDock->setWidget(editorDataView);
	addDockWidget(Qt::RightDockWidgetArea, editorDataDock);
	connect(scriptProxy, &ScriptProxy::editorDataChanged, editorDataView, &PlainTextEdit::setPlainText);

	// Create GUI
	createActions();
	createToolBars();
	createMenus();
	createStatusBar();

	connect(webView, SIGNAL(statusBarMessage(QString)), statusBar(), SLOT(showMessage(QString)));
	connect(webPage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(clearTools()));
	connect(webView, &QWebView::linkClicked, &QDesktopServices::openUrl);

	readSettings();

	setCurrentFile("");
	setUnifiedTitleAndToolBarOnMac(true);

	loadEditor("null.html");
}


/**
 * Attach this object to web page, so it is available from JavaScript.
 */
void MainWindow::attachToWebPage()
{
	scriptProxy->setObjectName("ScriptProxy");
	webView->page()->mainFrame()->addToJavaScriptWindowObject(QString("WebWrap"), scriptProxy);
}


void MainWindow::autodetectEditor()
{
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(scriptProxy->editorData().toUtf8(), &err);

	if (err.error != QJsonParseError::NoError) {
		QMessageBox msg;
		msg.setText(tr("The file does not contain a valid JSON data."));
		msg.setIcon(QMessageBox::Critical);
		msg.setInformativeText(tr("Error at offset %1: %2").arg(err.offset).arg(err.errorString()));
		msg.setStandardButtons(QMessageBox::Close);
		msg.exec();
		return;
	}

	if (!doc.isObject()) {
		QMessageBox msg;
		msg.setText(tr("The file does not contain a valid JSON object."));
		msg.setIcon(QMessageBox::Critical);
		msg.setStandardButtons(QMessageBox::Close);
		msg.exec();
		return;
	}

	QJsonObject docObject = doc.object();

	QJsonValue shebang = docObject.value("#!");
	if (shebang.isString()) {
		// Shebang exists, use it.
		QString shebangStr = shebang.toString();

		if (shebangStr == "block") {
			loadEditor("block.html");
		} else if (shebangStr == "smalldb_machine") {
			loadEditor("duf.html");
		} else if (shebangStr == "duf_form" || shebangStr == "duf_view") {
			loadEditor("duf.html");
		} else {
			loadEditor("plain.html");
			QMessageBox msg;
			msg.setText(tr("The file contains JSON data with unknown shebang."));
			msg.setIcon(QMessageBox::Warning);
			msg.setStandardButtons(QMessageBox::Ok);
			msg.exec();
		}
		return;
	}

	//// Heuristics when shebang is missing
	
	// Block
	if (docObject.value("blocks").isObject()) {
		loadEditor("block.html");
		return;
	}

	// Smalldb state machine
	if (docObject.value("states").isObject() && docObject.value("actions").isObject() && docObject.value("properties").isObject()) {
		loadEditor("smalldb.html");
		return;
	}

	// DUF form
	if (docObject.value("form").isObject() && docObject.value("field_groups").isObject() && docObject.value("layout").isObject()) {
		loadEditor("duf.html");
		return;
	}

	loadEditor("plain.html");
	QMessageBox msg;
	msg.setText(tr("The file contains JSON data of unknown structure."));
	msg.setIcon(QMessageBox::Warning);
	msg.setStandardButtons(QMessageBox::Ok);
	msg.exec();
}


void MainWindow::loadEditor(const QString &newGlueFileName)
{
	QString path;
	path.append(editorBaseDir);
	if (!path.endsWith('/')) {
		path.append('/');
	}
	path.append(newGlueFileName);
	glueFileName = newGlueFileName;
	webView->load(QUrl::fromLocalFile(path));
	webView->history()->clear();
	statusBar()->showMessage(tr("Loading editor (%1) ...").arg(path));
}

void MainWindow::reloadEditor()
{
	//loadEditor(glueFileName);
	QWebSettings::globalSettings()->clearMemoryCaches();
	webView->settings()->clearMemoryCaches();
	webView->triggerPageAction(QWebPage::ReloadAndBypassCache);
}

void MainWindow::unloadEditor()
{
	webView->load(QUrl("about:blank"));
	webView->history()->clear();
	scriptProxy->clear();
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
		unloadEditor();
		setCurrentFile("");
		loadEditor("null.html");
	}
}


void MainWindow::open()
{
	if (maybeSave()) {
		QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), curFile, tr("JSON files (*.json *.json.php);;All files (*)"));
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
	dialog.setDefaultSuffix("json.php");
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

	reloadEditorAct = new QAction(QIcon::fromTheme("view-refresh"), tr("&Reload editor"), this);
	QList<QKeySequence> reloadEditorShortcutList;
	reloadEditorShortcutList.append(QKeySequence(Qt::Key_F5));
	reloadEditorShortcutList.append(QKeySequence(Qt::CTRL + Qt::Key_R));
	reloadEditorAct->setShortcuts(reloadEditorShortcutList);
	reloadEditorAct->setStatusTip(tr("Reload the HTML editor without reloading the file"));
	connect(reloadEditorAct, SIGNAL(triggered()), this, SLOT(reloadEditor()));

	settingsAct = new QAction(QIcon::fromTheme("preferences-other"), tr("&Settings..."), this);
	settingsAct->setShortcuts(QKeySequence::Preferences);
	settingsAct->setStatusTip(tr("Open a settings dialog..."));
	connect(settingsAct, SIGNAL(triggered()), this, SLOT(openSettingsDialog()));

	undoAct = new QAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this);
	undoAct->setShortcuts(QKeySequence::Undo);
	undoAct->setStatusTip(tr("Undo recent action"));
	connect(undoAct, SIGNAL(triggered()), scriptProxy, SIGNAL(undo()));

	redoAct = new QAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this);
	redoAct->setShortcuts(QKeySequence::Redo);
	redoAct->setStatusTip(tr("Redo recent action"));
	connect(redoAct, SIGNAL(triggered()), scriptProxy, SIGNAL(redo()));

	cutAct = new QAction(QIcon::fromTheme("edit-cut"), tr("Cu&t"), this);
	cutAct->setShortcuts(QKeySequence::Cut);
	cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
	connect(cutAct, SIGNAL(triggered()), scriptProxy, SIGNAL(cut()));

	copyAct = new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
	copyAct->setShortcuts(QKeySequence::Copy);
	copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
	connect(copyAct, SIGNAL(triggered()), scriptProxy, SIGNAL(copy()));

	pasteAct = new QAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this);
	pasteAct->setShortcuts(QKeySequence::Paste);
	pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
	connect(pasteAct, SIGNAL(triggered()), scriptProxy, SIGNAL(paste()));

	tipsAct = new QAction(QIcon::fromTheme("help-contents"), tr("&Tips"), this);
	tipsAct->setShortcuts(QKeySequence::HelpContents);
	tipsAct->setCheckable(true);
	tipsAct->setStatusTip(tr("Show tips for better use of the editor"));
	connect(tipsAct, SIGNAL(triggered(bool)), scriptProxy, SIGNAL(tips(bool)));
	connect(scriptProxy, SIGNAL(tipsChecked(bool)), tipsAct, SLOT(setChecked(bool)));

	aboutAct = new QAction(QIcon::fromTheme("help-about"), tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	debugConsoleAct = webInspectorDock->toggleViewAction();
	debugConsoleAct->setIcon(QIcon::fromTheme("utilities-system-monitor"));
	debugConsoleAct->setCheckable(true);
	QList<QKeySequence> debugConsoleShortcutList;
	debugConsoleShortcutList.append(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
	debugConsoleShortcutList.append(QKeySequence(Qt::Key_F12));
	debugConsoleAct->setShortcuts(debugConsoleShortcutList);
	debugConsoleAct->setStatusTip(tr("Show web inspector"));



	editorDataViewAct = editorDataDock->toggleViewAction();
	editorDataViewAct->setIcon(QIcon::fromTheme("text-x-generic"));
	editorDataViewAct->setCheckable(true);
	editorDataViewAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
	editorDataViewAct->setStatusTip(tr("Show raw editor data"));

	cutAct->setEnabled(false);
	connect(scriptProxy, SIGNAL(cutAvailable(bool)), cutAct, SLOT(setEnabled(bool)));

	copyAct->setEnabled(false);
	connect(scriptProxy, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));

	pasteAct->setEnabled(false);
	connect(scriptProxy, SIGNAL(pasteAvailable(bool)), pasteAct, SLOT(setEnabled(bool)));

	undoAct->setEnabled(false);
	connect(scriptProxy, SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)));

	redoAct->setEnabled(false);
	connect(scriptProxy, SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)));

	tipsAct->setEnabled(false);
	connect(scriptProxy, SIGNAL(tipsAvailable(bool)), tipsAct, SLOT(setEnabled(bool)));

	connect(scriptProxy, SIGNAL(createToolAction(QString, QString, QString, QString, QString, bool, bool)),
			this, SLOT(createToolAction(QString, QString, QString, QString, QString, bool, bool)));
	connect(this, SIGNAL(toolAction(QString, bool)), scriptProxy, SIGNAL(toolAction(QString, bool)));
	connect(scriptProxy, SIGNAL(toolAvailable(QString, bool)), this, SLOT(toolAvailable(QString, bool)));
	connect(scriptProxy, SIGNAL(toolChecked(QString, bool)), this, SLOT(toolChecked(QString, bool)));
	connect(scriptProxy, SIGNAL(clearTools()), this, SLOT(clearTools()));
}


void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addSeparator();
	fileMenu->addAction(reloadEditorAct);
	fileMenu->addAction(settingsAct);
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

	toolMenu = menuBar()->addMenu(tr("&Tools"));
	toolMenu->setEnabled(false);

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(tipsAct);
	helpMenu->addSeparator();
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

	toolToolBar = addToolBar(tr("Tools toolbar"));
	toolToolBar->setObjectName("toolToolBar");

	debugToolBar = addToolBar(tr("Debug toolbar"));
	debugToolBar->setObjectName("debugToolBar");
	debugToolBar->addAction(debugConsoleAct);
	debugToolBar->addAction(editorDataViewAct);
	debugToolBar->hide();

	helpToolBar = addToolBar(tr("Help toolbar"));
	helpToolBar->setObjectName("helpToolBar");
	helpToolBar->addAction(tipsAct);
}


void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready."));
	
	statusProgressBar = new QProgressBar();
	statusProgressBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	statusBar()->addPermanentWidget(statusProgressBar, 0);

	connect(webView, &QWebView::loadStarted, statusProgressBar, &QProgressBar::show);
	connect(webView, &QWebView::loadProgress, statusProgressBar, &QProgressBar::setValue);
	connect(webView, &QWebView::loadFinished, this, &MainWindow::loadFinished);
}


void MainWindow::loadFinished(bool ok)
{
	statusProgressBar->hide();
	if (!ok) {
		statusBar()->showMessage(tr("Failed to load editor!"));
	}
}


QAction *MainWindow::createToolAction(const QString &name, const QString &label, const QString &iconName,
		const QString &shortcutKey, const QString &statusTip, bool isCheckable, bool isOnToolbar)
{
	QAction *act = new QAction(QIcon::fromTheme(iconName), label, this);
	act->setObjectName(name);
	act->setEnabled(false);
	act->setShortcut(QKeySequence(shortcutKey));
	if (!statusTip.isEmpty()) {
		act->setStatusTip(statusTip);
	} else {
		act->setStatusTip(label);
	}
	act->setCheckable(isCheckable);
	if (toolActions[name]) {
		delete toolActions[name];
	}
	toolActions[name] = act;

	toolMenu->addAction(act);
	if (isOnToolbar) {
		toolToolBar->addAction(act);
	}

	connect(act, SIGNAL(triggered(bool)), this, SLOT(toolActionTriggered(bool)));

	toolMenu->setEnabled(true);

	return act;
}

void MainWindow::clearTools()
{
	QHashIterator<QString, QAction*> i(toolActions);
	while (i.hasNext()) {
		i.next();
		delete i.value();
	}
	toolActions.clear();
}

void MainWindow::toolActionTriggered(bool isChecked)
{
	QObject *src = sender();
	emit toolAction(src->objectName(), isChecked);
}

void MainWindow::toolAvailable(const QString &name, bool isAvailable)
{
	QAction *act = toolActions.value(name);
	if (act) {
		act->setEnabled(isAvailable);
	}
}


void MainWindow::toolChecked(const QString &name, bool isChecked)
{
	QAction *act = toolActions.value(name);
	if (act) {
		act->setChecked(isChecked);
	}
}


void MainWindow::openSettingsDialog()
{
	writeSettings();
	SettingsDialog sd(this);
	if (sd.exec()) {
		readSettings();
		webView->reload();
	}
}

void MainWindow::readSettings()
{
	QSettings settings("WebWrapEditor", "WebWrapEditor");
	settings.beginGroup("htmlEditors");
	editorBaseDir = settings.value("baseDir", DEFAULT_EDITOR_BASE_DIR).toString();
	settings.endGroup();
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
	if (scriptProxy->isModified()) {
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
	unloadEditor();
	scriptProxy->setFileName(fileName);
	scriptProxy->setEditorData(in.readAll());
	scriptProxy->setModified(false);

#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif
	setCurrentFile(fileName);
	//statusBar()->showMessage(tr("File loaded"), 2000);

	autodetectEditor();
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
	if (curFile.isEmpty()) {
		shownName = "untitled.json";
	}
	setWindowFilePath(shownName);
}


QString MainWindow::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}


