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

#include <QSettings>

#include "settingsdialog.h"
#include "build/ui_settingsdialog.h"


SettingsDialog::SettingsDialog(QWidget *parent)
	: QDialog(parent), ui(new Ui_SettingsDialog)
{
	ui->setupUi(this);

	QSettings settings("WebWrapEditor", "WebWrapEditor");
	settings.beginGroup("htmlEditors");

	ui->htmlEditorsBaseDirLineEdit->setPlaceholderText(DEFAULT_EDITOR_BASE_DIR);
	ui->htmlEditorsBaseDirLineEdit->setText(settings.value("baseDir", DEFAULT_EDITOR_BASE_DIR).toString());

	settings.endGroup();

	connect(this, SIGNAL(accepted()), this, SLOT(writeSettings()));
}

void SettingsDialog::writeSettings()
{
	QSettings settings("WebWrapEditor", "WebWrapEditor");
	settings.beginGroup("htmlEditors");

	QString baseDir = ui->htmlEditorsBaseDirLineEdit->text();
	if (baseDir == "" || baseDir == DEFAULT_EDITOR_BASE_DIR) {
		settings.remove("baseDir");
	} else {
		settings.setValue("baseDir", baseDir);
	}

	settings.endGroup();
}


SettingsDialog::~SettingsDialog()
{
	delete ui;
}

