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

#include <QApplication>
#include <QCommandLineParser>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
	//Q_INIT_RESOURCE(application);

	QApplication app(argc, argv);
	app.setOrganizationName("WebWrapEditor");
	app.setApplicationName("WebWrapEditor");
	app.setApplicationVersion(GIT_VERSION);

	QCommandLineParser parser;
	parser.setApplicationDescription("Test helper");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("filename", QCoreApplication::translate("main", "File to edit"));
	parser.process(app);

	const QStringList filenames = parser.positionalArguments();
	if (filenames.length() > 1) {
		parser.showHelp();
		return -1;
	}

	MainWindow mainWin;
	if (filenames.length() == 1) {
		if (!mainWin.loadFile(filenames.at(0))) {
			return -1;
		}
	}
	mainWin.show();

	return app.exec();
}

