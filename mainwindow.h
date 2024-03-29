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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>

class QAction;
class QMenu;
class QPlainTextEdit;
class QWebView;
class QWebInspector;
class QDockWidget;
class QProgressBar;
class ScriptProxy;
class PlainTextEdit;


class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		MainWindow();

	protected:
		void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

	public slots:
		bool loadFile(const QString &fileName);
		bool saveFile(const QString &fileName);

	private slots:
		void newFile();
		void open();
		bool save();
		bool saveAs();
		void about();
		void documentWasModified();
		void attachToWebPage();

		void autodetectEditor();
		void loadEditor(const QString &glueFileName);
		void reloadEditor();
		void unloadEditor();
		void loadFinished(bool ok);

		void openSettingsDialog();

		QAction *createToolAction(const QString &name, const QString &label, const QString &iconName,
				const QString &shortcutKey, const QString &statusTip, bool isCheckable, bool isOnToolbar);
		void toolActionTriggered(bool isChecked = false);
		void toolAvailable(const QString &name, bool isAvailable);
		void toolChecked(const QString &name, bool isChecked);
		void clearTools();

	signals:
		void toolAction(const QString &name, bool isChecked);

	private:
		void createActions();
		void createMenus();
		void createToolBars();
		void createStatusBar();
		void readSettings();
		void writeSettings();
		bool maybeSave();
		void setCurrentFile(const QString &fileName);
		QString strippedName(const QString &fullFileName);

		QWebView *webView;
		QDockWidget *webInspectorDock;
		QWebInspector *webInspector;
		ScriptProxy *scriptProxy;

		PlainTextEdit *editorDataView;
		QDockWidget *editorDataDock;

		QString curFile;
		QString editorBaseDir;

		QMenu *fileMenu;
		QMenu *editMenu;
		QMenu *viewMenu;
		QMenu *toolMenu;
		QMenu *helpMenu;
		QToolBar *fileToolBar;
		QToolBar *editToolBar;
		QToolBar *toolToolBar;
		QToolBar *debugToolBar;
		QToolBar *helpToolBar;
		QAction *newAct;
		QAction *openAct;
		QAction *saveAct;
		QAction *saveAsAct;
		QAction *reloadEditorAct;
		QAction *settingsAct;
		QAction *exitAct;
		QAction *undoAct;
		QAction *redoAct;
		QAction *cutAct;
		QAction *copyAct;
		QAction *pasteAct;
		QAction *tipsAct;
		QAction *aboutAct;
		QAction *aboutQtAct;
		QAction *debugConsoleAct;
		QAction *editorDataViewAct;

		QHash<QString, QAction*> toolActions;

		QProgressBar *statusProgressBar;
		QString glueFileName;
};

#endif

