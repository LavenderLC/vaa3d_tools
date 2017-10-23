/* Dataset_Generator_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2017-9-26 : by MK
 */
 
#ifndef __DATASET_GENERATOR_PLUGIN_H__
#define __DATASET_GENERATOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <qvector.h>
#include "basic_surf_objs.h"

class DatasetGenerator : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
	
/*-------------------------------------------------------------------------------------------------------------------------------------------------*/
	DatasetGenerator();
	
	QString inputSWCdir;
	QString inputImagedir;
	QString inputBkgDir;
	QVector<QString> imageFolders;
	QString outputImagedir;
	void getImageFolders();
	
	double valProportion;
	double proportion1;
	double proportion2;
	void createList();
	void createList2();
	

	void pick_save();

};

#endif

