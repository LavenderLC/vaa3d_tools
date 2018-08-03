﻿//pre_processing_main.cpp
//main function for pre processing (blastneron)
//by Yinan Wan
//2012-06-02
//modified by Peng Xie
//2018-07-27

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "pre_processing_main.h"
#include "basic_surf_objs.h"
//#include "sort_eswc.h"
#include "sort_swc.h"
#if !defined(Q_OS_WIN32)
#include <unistd.h>
#endif

#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include <io.h>
#endif

bool pre_processing_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{

	printf("welcome to pre_processing\n");


	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist = NULL;
	vector<char*>* paralist = NULL;

	if(input.size() != 2) 
	{
		printf("Please specify parameter set.\n");
		printHelp_pre_processing();
		return false;
	}
	paralist = (vector<char*>*)(input.at(1).p);


	if (paralist->size()!=1)
	{
		printf("Please specify all paramters in one text string.\n");
		printHelp_pre_processing();
		return false;
	}
	

    char * paras = paralist->at(0);
    int argc = 1;
    enum {kArgMax = 64};
    char *argv[kArgMax];
	
	//parsing parameters
	if (paras)
	{
		int len = strlen(paras);
		for (int i=0;i<len;i++)
		{
			if (paras[i]=='#')
				paras[i] = '-';
		}

        char* pch = strtok(paras, " ");
		while (pch && argc<kArgMax)
        {
            argv[argc++] = pch;
            pch = strtok(NULL, " ");
		}
	}
	else
		printHelp_pre_processing();


    //1. read arguments

	char *dfile_input = NULL;
	char *dfile_result = NULL;
	char *outfile = NULL;
	double step_size = 2;
    double thres = 2;
    double prune_size = -1; //default case
	int skip_rotation = 1;
	
	int c;
    static char optstring[]="h:i:o:l:s:t:r:";
	extern char * optarg;
	extern int optind, opterr;
    optind = 1;
    while ((c = getopt(argc, argv, optstring))!=-1)
    {
        switch (c)
		{
			case 'h':
				printHelp_pre_processing();
				return 0;
				break;
			case 'i':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				dfile_input = optarg;
				break;
			case 'o':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				dfile_result = optarg;
				break;

             case 'l':
                 if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -l.\n");
                    return 1;
                }
                prune_size = atof(optarg);
                if (prune_size<-1)
                {
                    fprintf(stderr, "Illegal prune size. Special case: 0 -- no prunning; -1 -- default prunning size.\n");
                }
                  break;
            case 's':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				step_size = atof(optarg);
                if (step_size<0)
                {
                    fprintf(stderr, "Illegal step_size.\n");
                }
                break;
            case 't':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
                    return 1;
                }
                thres = atof(optarg);
                if (thres<-1)
                {
                    fprintf(stderr, "Illegal thres.\n");
                }
                break;
           case 'r':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
                    return 1;
                }
                skip_rotation = atoi(optarg);
                break;
			case '?':
				fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
				return 1;
				break;
		}
	}

    QString qs_input(dfile_input);
	NeuronTree nt = readSWC_file(qs_input);
	QString outfileName = QString(dfile_result);
	if (dfile_result==NULL)
	{
		outfileName = qs_input+"_preprocessed.swc";
	}

    //2. start processing
    //2.1 Prune
	printf("Pruning short branches\n");
	NeuronTree pruned;

    if (!prune_branch(nt, pruned, prune_size))
	{
		fprintf(stderr,"Error in prune_short_branch.\n");
		return 1;
	}

    //2.2 Remove duplicates
    QList<NeuronSWC> newNeuron;
    NeuronTree deduped;
    SortSWC(pruned.listNeuron, deduped.listNeuron, VOID, 0.25);
//    connect_swc(pruned, deduped.listNeuron, 100, 90);
    export_listNeuron_2swc(deduped.listNeuron,qPrintable(QString("01_test.swc")));

//	QString fileOpenName = nt.file;
//	(SortSWC(neuron, result ,rootid, thres))

    //2.3 Resample
    NeuronTree resampled = deduped;
    if (step_size>0){
        printf("Resampling along segments\n");
        resampled = resample(pruned, step_size);
    }else{
        printf("Skip Resampling\n");
        resampled=pruned;
    }    

    //2.4 Sort
    //The old version of sort function is slow for large (>20,000 nodes) trees
    //Replaced by neuron_connector
    NeuronTree sorted;
    if (thres>0){
        printf("Sort \n");
//        sorted = sort(resampled,VOID, thres); // Original 2012-06-02
        QString resamplefileName = qs_input + ".resample.temp.swc";
        export_listNeuron_2swc(resampled.listNeuron, qPrintable(resamplefileName));
        QString sortedfileName = qs_input + ".sorted.temp.swc";
        // Call neuron_connector
        printf("Running neuron_connector...\n");
//        QString sys_cmd = QString("vaa3d -x neuron_connector -f connect_neuron_SWC -i %1 -o %2 -p 60 10 5 1 1").arg(resamplefileName.toStdString().c_str()).arg(sortedfileName.toStdString().c_str());
        QString subStr("swc");
        QString newStr("apo");
        QString old_apo = qs_input;
        QString new_apo = resamplefileName;
        old_apo.replace(old_apo.lastIndexOf(subStr), subStr.size(), newStr);
        new_apo.replace(new_apo.lastIndexOf(subStr), subStr.size(), newStr);
        QString apo_cmd = QString("cp %1 %2").arg(old_apo.toStdString().c_str()).arg(new_apo.toStdString().c_str());
        system(qPrintable(apo_cmd));
        printf(qPrintable(resamplefileName));
        QString sys_cmd = QString("vaa3d -x sort_neuron_swc_lmg -f sort_swc_lmg -i %1 -o %2 -p 1000").arg(resamplefileName.toStdString().c_str()).arg(sortedfileName.toStdString().c_str());
        system(qPrintable(sys_cmd));
        sorted = readSWC_file(sortedfileName);
        // Cleanup
        sys_cmd = QString("rm %1").arg(resamplefileName.toStdString().c_str());
//        system(qPrintable(sys_cmd));
        sys_cmd = QString("rm %1").arg(sortedfileName.toStdString().c_str());
//        system(qPrintable(sys_cmd));
    }else{
        printf("Skip sorting\n");
        sorted=resampled;
    }

    //2.4 Align axis
	NeuronTree result;
	if (skip_rotation!=1)
	{
		printf("Aligning PCA axis\n");
        result = align_axis(sorted);
	}
    else{
        printf("Skip PCA alignment\n");
        result = sorted;
    }

    if (export_listNeuron_2swc(result.listNeuron,qPrintable(outfileName)))
		printf("\t %s has been generated successfully.\n",qPrintable(outfileName));

	return 1;
}

void printHelp_pre_processing()
{
    printf("\nVaa3D plugin: modifiled pre-processing step for BlastNeuron pipeline, including: \n");
    printf("\t1) delete the branches in a neuron which have a length smaller the prune size ( by default the prune size is 5 %% of neuron/tree diameter.\n");
    printf("\t2) resample the neurons along the segments with the step size (default: 1). \n");
    printf("\t3) sort the neurons along the segments with the step size (deyyplt: 1). \n");
    printf("\t4) rotate the neuron to align its longest dimension to x axis\n");
	printf("\t#i <neuron_filename> :   input neuron structure (.swc) name\n");
	printf("\t#o <output_filename> :   output file name.\n");
	printf("\t                         if not specified, it is \"inputName_preprocessed.swc\"\n");
    printf("\t#l <prune_size> :  prune short branches that has length smaller than prune_size.\n");
    printf("\t                         if not specified, it is \"inputName_preprocessed.swc\"\n");
	printf("\t#s <step_size>       :   step size for resampling.\n");
    printf("\t                         use 0 to skip, if not specified, use 2.\n");
    printf("\t#t <thres>       :    gap threshold for connecting during the sorting procedure.\n");
    printf("\t                         use 0 to skip, if not specified, use 2.\n");
    printf("\t#r <skip_rotation_flag = 1>   :   whether to skip PCA alignment.\n");
    printf("\t                         if not specified, rotation is not performed\n");

    printf("Usage: vaa3d -x preprocess -f preprocess -p \"#i input.swc #o result.swc #l 3 #s 2 #t 2 #r 0\"\n");
}
