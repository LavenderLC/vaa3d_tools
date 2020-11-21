#include <sstream>
#include <fstream>
#include <cstdio>

#include <qtextstream.h>
#include <qstring.h>
#include <qdir.h>

#include "swcRenameDlg.h"

using namespace std;

SWC_renameDlg::SWC_renameDlg(QWidget* parent, V3DPluginCallback2* callback) : uiPtr(new Ui_dialog), thisCallback(callback), QDialog(parent)
{
	uiPtr->setupUi(this);
	
	this->fileManager.connToken.push_back("_");
	this->fileManager.connToken.push_back("-");
	this->fileManager.connToken.push_back("-");
	this->fileManager.connToken.push_back("_");

	this->fileManager.fileNameAddition.clear();

	this->show();
}

void SWC_renameDlg::browseFolderClicked()
{
	QObject* signalSender = sender();
	QString objName = signalSender->objectName();

	if (objName == "pushButton")
		uiPtr->lineEdit->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog));
	else if (objName == "pushButton_3")
		uiPtr->lineEdit_2->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog));
	else if (objName == "pushButton_5")
		uiPtr->lineEdit_4->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog));
	else if (objName == "pushButton_6")
		uiPtr->lineEdit_9->setText(QFileDialog::getOpenFileName(this, tr("Choose the mapping table file"), ""));
	else if (objName == "pushButton_7")
		uiPtr->lineEdit_10->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog));
	else if (objName == "pushButton_8")
		uiPtr->lineEdit_11->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog));
}

void SWC_renameDlg::changeName()
{
	QObject* signalSender = sender();
	QString objName = signalSender->objectName();

	if (objName == "buttonBox")
	{	
		this->rootPath = uiPtr->lineEdit->text();
		this->rootPath.replace("/", "\\");
		this->fileManager.rootPath = this->rootPath;
		this->fileManager.fileNameAddition = uiPtr->lineEdit_8->text();

		QDir swcFolder(this->rootPath);
		swcFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		this->fileNameList.clear();
		this->fileNameList = swcFolder.entryList();

		this->oldNewMap.clear();
		if (uiPtr->radioButton->isChecked()) this->fileManager.WMUapoAno = true;
		else this->fileManager.WMUapoAno = false;
		this->fileManager.nameChange(this->fileNameList, FileNameChangerIndexer::WMU_name, &(this->oldNewMap));
	}
	else if (objName == "buttonBox_2")
	{
		this->rootPath = uiPtr->lineEdit_2->text();
		this->rootPath.replace("/", "\\");
		this->fileManager.rootPath = this->rootPath;
		this->fileManager.fileNameAddition = uiPtr->lineEdit_8->text();
		this->fileManager.mappingTableFullName = uiPtr->lineEdit_9->text();

		QDir seuFileFolder(this->rootPath);
		seuFileFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		this->fileNameList.clear();
		this->fileNameList = seuFileFolder.entryList();

		this->fileManager.nameChange(this->fileNameList, FileNameChangerIndexer::SEU_index);
	}
}

void SWC_renameDlg::reconOp()
{
	QObject* signalSender = sender();
	QString objName = signalSender->objectName();

	if (objName == "buttonBox_4")
	{
		this->rootPath = uiPtr->lineEdit_4->text();
		this->rootPath.replace("/", "\\");
		this->myOperator.rootPath = this->rootPath;

		QDir seuFileFolder(this->rootPath);
		seuFileFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		this->fileNameList.clear();
		this->fileNameList = seuFileFolder.entryList();

		float xFactor = 1 / (uiPtr->lineEdit_5->text().toFloat());
		float yFactor = 1 / (uiPtr->lineEdit_6->text().toFloat());
		float zFactor = 1 / (uiPtr->lineEdit_7->text().toFloat());

		this->myOperator.downSampleReconFile(this->fileNameList, xFactor, yFactor, zFactor);
	}
	else if (objName == "buttonBox_5")
	{
		this->rootPath = uiPtr->lineEdit_10->text();
		this->rootPath.replace("/", "\\");
		this->myOperator.rootPath = this->rootPath;

		QDir inputFileFolder(this->rootPath);
		inputFileFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		this->fileNameList.clear();
		this->fileNameList = inputFileFolder.entryList();

		this->myOperator.denAxonSeparate(this->fileNameList);
	}
	else if (objName == "buttonBox_6")
	{
		this->rootPath = uiPtr->lineEdit_11->text();
		this->rootPath.replace("/", "\\");
		this->myOperator.rootPath = this->rootPath;

		this->myOperator.denAxonCombine();
	}
}

void SWC_renameDlg::undoClicked()
{
	QString lineText = uiPtr->lineEdit->text();
	lineText.replace("/", "\\");

	if (this->oldNewMap.empty())
	{
		v3d_msg("No file names have been changed in this folder yet.");
		return;
	}
	else if (lineText != this->rootPath)
	{
		v3d_msg("You have changed to a different folder.");
		return;
	}

	for (auto& newNamePair : this->oldNewMap)
	{
		string oldName = this->rootPath.toStdString() + "/" + newNamePair.first;
		const char* oldNameC = oldName.c_str();
		string newName = this->rootPath.toStdString() + "/" + newNamePair.second;
		const char* newNameC = newName.c_str();

		rename(newNameC, oldNameC);
		cout << newNamePair.second << " -> " << newNamePair.first << endl;
		this->oldNewMap[newNamePair.first] = "";
	}
}