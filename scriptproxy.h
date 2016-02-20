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

	private:
		bool m_modified;
		QString m_fileName;
		QString m_editorData;

};

#endif

