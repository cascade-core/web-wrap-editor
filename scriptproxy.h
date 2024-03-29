/*
 * Copyright (c) 2016  Josef Kufner <josef@kufner.cz>
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
#ifndef SCRIPT_PROXY_H
#define SCRIPT_PROXY_H

#include <QObject>

class QAction;

/**
 * Proxy between Javascript in WebView (available as `WebWrap`) and MainWindow.
 */
class ScriptProxy : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool modified READ isModified WRITE setModified NOTIFY isModifiedChanged)
	Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
	Q_PROPERTY(QString editorData READ editorData WRITE setEditorData NOTIFY editorDataChanged)

	public:
		ScriptProxy(QObject *parent);
		bool isModified() { return m_modified; };
		const QString fileName() { return m_fileName; };
		const QString editorData() { return m_editorData; };

	public slots:
		void clear();
		void setModified(bool modified);
		void setFileName(const QString &fileName);
		void setEditorData(const QString &editorData);

	signals:
		void isModifiedChanged(bool);
		void fileNameChanged(const QString &);
		void editorDataChanged(const QString &);
		void cut();
		void copy();
		void paste();
		void undo();
		void redo();
		void tips(bool);
		void cutAvailable(bool);
		void copyAvailable(bool);
		void pasteAvailable(bool);
		void undoAvailable(bool);
		void redoAvailable(bool);
		void tipsAvailable(bool);
		void tipsChecked(bool);

		void createToolAction(const QString &name, const QString &label, const QString &iconName,
				const QString &shortcutKey, const QString &statusTip, bool isToggle, bool isOnToolbar);
		void toolAction(const QString &name, bool isChecked);
		void toolAvailable(const QString &name, bool isAvailable);
		void toolChecked(const QString &name, bool isAvailable);
		void clearTools();

	private:
		bool m_modified;
		QString m_fileName;
		QString m_editorData;

};

#endif

