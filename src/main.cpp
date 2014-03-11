////////////////////////////////////////////////////////////////////////////////
// Daniel Smith
// dsmith@ics.mq.edu.au
////////////////////////////////////////////////////////////////////////////////
// Program to test Named Entity Recognition
// For training and testing.
// Run with parameters:
// e.g.  afner --train-mode -M modelL235.mdl -T ./RemediaAnnot/level2/ -T ./RemediaAnnot/level3/ -T ./RemediaAnnot/level5/ -D d:/afner/resultL2345.dmp
//  or  afner --run-mode -M data/model-RC-L235.mdl -R RemediaPlain/level4 -O results/
////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2008  Diego Molla-Aliod <diego@ics.mq.edu.au>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <dirent.h>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "tokeniser.h"
#include "ner.h"
#include "entity_tag.h"
#include "regex_handler.h"
#include "list_handler.h"

using namespace std;
using namespace AF;
using namespace boost::program_options;
namespace fs=boost::filesystem; 

StringXML readfile(const StringXML& fname);
void list_directory(const StringXML& path, vector<StringXML>& flisting,
    vector<StringXML>& dirlisting);
void test(const vector<StringXML>& files,
    const vector<StringXML>& dirs,NEDeco deco,
    const StringXML& format,const StringXML resultsDir, float threshold);
void testDirectory(const StringXML& path,NEDeco deco,const StringXML format,
    const StringXML resultsDir,float threshold);
void testFile(const StringXML& path,const StringXML& fname,
    NEDeco deco,const StringXML format,
    const StringXML resultsDir, float threshold);
   
void train(const StringXML& outputFile, const vector<StringXML>& files,
    const vector<StringXML>& dirs,NEDeco deco,const int classCount,
    const StringXML& freqFile,const StringXML& prevFreqFile);
void trainDirectory(const StringXML& path,ostream& out,
    NEDeco deco,vector<map<StringXML,int> >& frequencies,
    vector<map<StringXML,int> >& prevFrequencies,
    const bool countFrequencies);
void trainFile(const StringXML& path,ostream& out,
    NEDeco deco, vector<map<StringXML,int> >& frequencies,
    vector<map<StringXML,int> >& prevFrequencies,
    const bool countFrequencies);

void setFeatureWeight(const StringXML& setting, FeatureHandler& fh);

map<StringXML,EntityTag>
InitializeListHandler(const StringXML& file);

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v) {
  copy(v.begin(), v.end(), ostream_iterator<T>(cout, " "));
  return os;
}

int main(int argc, char* argv[]) {
  vector<StringXML> files;
  vector<StringXML> dirs;
  vector<StringXML> feature_weight;
  StringXML mode = "";
  StringXML listfiles = "";  
  StringXML trainDataFile="";
  StringXML modelFile="";
  StringXML regex_loc="";
  StringXML feature_regex_file="";
  StringXML outputLocation="";
  StringXML freq="";
  StringXML prevFreq="";
  StringXML freqIn="";
  StringXML prevFreqIn="";
  int context=0;
  StringXML format="NORMAL";
  int maxLabels=1;
  StringXML configFile="";
  bool dotest=false;
  bool dotrain=false;
  bool docount=false;
  bool dodump=false;
  bool singleLabels = false;
  bool insufParam = false;
  StringXML tagsetLoc = "";
  float threshold;
  float default_weight;

  try {
    // options only for command line
    options_description command("Command line only arguments");
    command.add_options()
      ("help,h","produce this help message")
      ("config-file,c",value<StringXML>(&configFile),
          "file containing configuration settings")
    ;
    // options for counting tokens
    options_description counting("Settings for counting tokens");
    counting.add_options()
      ("count", "Set mode for counting")
      ("token-frequency-output",value<StringXML>(&freq),
       "file to dump token frequencies to")
      ("prev-token-frequency-output",value<StringXML>(&prevFreq),
           "file to dump previous token frequencies to")
    ;
    // options for training
    options_description training("Training settings");
    training.add_options()
      ("train", "Set mode for training")
      ("training-data-file,D",value<StringXML>(&trainDataFile),
           "file to dump training data to")
      ("output-model-file",value<StringXML>(&modelFile),
           "model file to generate")
    ;
    // options for running
    options_description running("Running settings");
    running.add_options()
      ("run", "Set mode for running (testing)")
      ("model-file,M",value<StringXML>(&modelFile)->default_value("config/BBN.mdl"),
           "model file to read")
      ("format,F",value<StringXML>(&format),
            "specify the format of the"
            "output as either NORMAL or SHORT")
      ("max-labels,L",value<int>(&maxLabels)->default_value(3),
            "maximum labels to assign to a"
            "token")
      ("single,S","allow only a single tag per"
            "token (Default is multiple tags)")
      ("threshold,e",value<float>(&threshold)->default_value(0.0),
            "threshold above which a NE will be issued")
      ("output-path,O",
          value<StringXML>(&outputLocation)->default_value("afner-output"),
            "path to dump output from NER runs")
    ;
    // options for dumping NER input data
    options_description dumping("Data dumping settings");
    dumping.add_options()
      ("dump", "Set mode for generating Yasmet input data only")
      ("training-data-file,D",value<StringXML>(&trainDataFile),
           "file to dump Yasmet input data to")
    ;
    // common options
    options_description config("Common options");
    config.add_options()
      ("path,P",value< vector<StringXML> > (&dirs)->composing(),
       "directory of files")
      ("file,f",value< vector<StringXML> >(&files)->composing(),
       "individual file")
      ("context,C",value<int>(&context)->default_value(2),
           "range of contextual features used")
      ("entity_list",value<StringXML>(&listfiles)->default_value("config/list_spec"),
          "file containing location of entity lists paired with tags")
      ("tagset,g",value<StringXML>(&tagsetLoc)->default_value("config/BBN_tags"),
           "location of file storing tagset information")
      ("regex-file,x",value<StringXML>(&regex_loc)->default_value("config/regex"),
          "location of broad regular expression specification")
      ("feature-regex-file,y",value<StringXML>(&feature_regex_file)->default_value("config/feature_regex"),
          "location of regular expressions to be used only as features")
      ("feature-weight",value< vector<StringXML> >(&feature_weight)->
          composing()->multitoken(),
          "sets the weight of a feature i.e. feature1 0")
      ("default-feature-weight",
          value<float>(&default_weight)->default_value(1.0),
          "sets the default weight of features")
      ("token-frequency-input", value<StringXML>(&freqIn),
          "the location of token frequency information generated in training")
      ("prev-token-frequency-input", value<StringXML>(&prevFreqIn),
          "the location of previous token frequency information generated in"
          " training")
    ;

    options_description cmdline_options;
    cmdline_options.add(command).add(config).add(running).add(training).add(dumping).add(counting);

    options_description config_file_options;
    config_file_options.add(config).add(running).add(training).add(dumping).add(counting);
    
    positional_options_description p;
    p.add("run-file",-1);
  
    variables_map vm;
    store(command_line_parser(argc,argv).
	  options(cmdline_options).positional(p).run(),vm);
    notify(vm);
    ifstream ifs(configFile.c_str());
    store(parse_config_file(ifs,config_file_options),vm);
    
    notify(vm);
    
    // if help option selected, print help
    if (vm.count("help")) {
      cmdline_options.print(cout);
      cout << endl << "Configuration options can be set either at " 
           << "command line or in the configuration \nfile." << endl << endl;
      return 1;
    }
    
    // work out from passed parameters whether to test, train or count
    int countModes = 0;
    if (vm.count("count")) {
      docount = true;
      countModes += 1;
      cout << "Counting parameters detected" << endl;
    }
    
    if (vm.count("train")) {
      dotrain = true;
      countModes += 1;
      cout << "Training parameters detected" << endl;
    }

    if (vm.count("run")) {
      dotest = true;
      countModes += 1;
      cout << "Testing parameters detected." << endl;
    }

    if (vm.count("dump")) {
      dodump = true;
      countModes += 1;
      cout << "Dumping parameters detected." << endl;
    }

    if (countModes > 1) {
      cout << "Only one mode (count, train, test, dump is allowed)." << endl;
      insufParam = true;
    }

    if (countModes == 0) {
      dotest = true;
      cout << "No mode selected; defaulting to --run (testing parameters)" 
	   << endl;
    }

    if (vm.count("path") + vm.count("file") == 0) {
      cout << "You must specify at least a directory (-P) and/or a file (-f)."
	   << endl;
      insufParam = true;
    }

    if ((dotrain || dodump) && vm.count("training-data-file") == 0) {
      cout << "You must specify a training data file (-D)."
	   << endl;
      insufParam = true;
    }

    if (dotrain && vm.count("output-model-file") == 0) {
      cout << "You must provide a model file in the training mode." << endl 
	   << "Use --dump instead of --train if you only want to produce the "
	   << "Yasmet input data." 
	   << endl;
      insufParam = true;
    }

    if ((dotrain || dotest || dodump) && vm.count("tagset") == 0) {
      cout << "You must specify a tagset (-g)."
	   << endl;
      insufParam = true;
    }

    if (insufParam) {
      cout << "Run with --help for options."
	   << endl;
      return 1;
    }

    // initialise tagset
    ifstream inFile;
    inFile.open(tagsetLoc.c_str());
    if (!inFile) {
      cout << "Unable to open tagset data file " << tagsetLoc << endl;
      return 1;
    }
    EntityTagset t(inFile);
    inFile.close();
    cout << endl << "Using tags:" << endl;
    t.printTags(cout);
    cout << endl;
    RegexHandler rh;
    if (regex_loc!="") {
      ifstream inFile;
      inFile.open(regex_loc.c_str());
      if (!inFile) {
	cout << "Unable to open regex data file " << regex_loc << endl;
      }
      RegexHandler r(inFile);
      rh = r;
      inFile.close();
    }
    if (listfiles=="") {
      cout << "Warning: no lists specified." << endl;
    }
    // if lists have been specified
    ListHandler lh(InitializeListHandler(listfiles));
    // create the feature handler
    FeatureHandler fh(lh,rh,t,feature_regex_file,context,
		      default_weight,freqIn,prevFreqIn);
    // set custom feature weights
    if (vm.count("feature-weight")) {
      for (vector<StringXML>::const_iterator i=feature_weight.begin();
	   i!=feature_weight.end();i++) {
	setFeatureWeight(*i,fh);
      }
    }
    cout << endl;
    fh.printFeatures(cout);
    cout << endl;
    // create the decorator object
    NEDeco deco(&t,&rh,&lh,&fh,modelFile,maxLabels,context,singleLabels);
    if (docount) {
      cout << "COUNTING" << endl << endl;
      train(trainDataFile,files,dirs,deco,t.classCount(),freq,
            prevFreq);
    }
    if (dotrain) {
      cout << "TRAINING" << endl << endl;
      train(trainDataFile,files,dirs,deco,t.classCount(),freq,
            prevFreq);
      // if a model file is given, create the model file directly
      if (vm.count("model-file")) {
	cout << "Generating model file. This may take some time..."
	     << endl << endl;
	stringstream ss;
	ss << "cat " << trainDataFile << " | " << "../yasmet/yasmet > "
	   << modelFile;
	StringXML cmd = ss.str();
	std::system(cmd.c_str());
      }
    }
    if (dodump) {
      cout << "DUMPING" << endl << endl;
      train(trainDataFile,files,dirs,deco,t.classCount(),freq,
            prevFreq);
    }
    if (dotest) {
      cout << "TESTING" << endl;
      test(files,dirs,deco,format,outputLocation,threshold);
    }
  }
  catch(std::exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  }
  catch(...) {
    cerr << "Unknown exception." << endl;
  }                   
  return EXIT_SUCCESS;
}

void testDirectory(const StringXML& path,NEDeco deco,
                   const StringXML format,
                   const StringXML resultsDir,
                   float threshold){
  vector<StringXML> flist;
  vector<StringXML> dirlist;
  list_directory(path,flist,dirlist);
  cout << "Running directory: " << path << endl;
  cout << "Output to: " << resultsDir << endl << endl;
  for (unsigned int i=0;i<flist.size();i++){
    //cout << "Testing File " << path << flist[i] << " " << resultsDir << endl;
    testFile(path,flist[i],deco,format,resultsDir,threshold);
    cout << i + 1 << " of " << flist.size() << " files done" << endl << endl;
  }
  for (unsigned int i=0;i<dirlist.size();i++) {
    StringXML newpath = path + "/" + dirlist[i];
    StringXML newres = resultsDir + "/" + dirlist[i];
    //cout << "testDirectory " << newpath << " " << newres << endl;
    testDirectory(newpath,deco,format,newres,threshold);
    cout << i + 1 << " of " << dirlist.size() 
        << " directories done" << endl << endl;
  }
}

void trainDirectory(const StringXML& path,ostream& out,NEDeco deco,
    vector<map<StringXML,int> >& frequencies,
    vector<map<StringXML,int> >& prevFrequencies,
    const bool countFrequencies){
  vector<StringXML> flist;
  vector<StringXML> dirlist;
  list_directory(path,flist,dirlist);
  cout << path << endl;
  for (unsigned int i=0;i<flist.size();i++) {
    cout << '_' << flush;
  }
  cout << endl;
  for (unsigned int i=0;i<flist.size();i++){
    StringXML full = path + "/" + flist[i];
    trainFile(full,out,deco,frequencies,prevFrequencies,countFrequencies);
    cout << '|' << flush;
  }
  cout << endl << endl;
}

// given a model file, do the Entity checking
void testFile(const StringXML& path,const StringXML& fname,
    NEDeco deco,const StringXML format,
    const StringXML resultsDir, float threshold) {
  StringXML fullname = path + "/" + fname;
  StringXML simpFilename=fname;
  if (path==".") {
    fullname = fname;
    int slashpos=fname.rfind("/",fname.size()-1);
    simpFilename=fname.substr(slashpos,fname.size());
  }
  StringXML text = readfile(fullname);
  StringXML resultfile = resultsDir + "/" + simpFilename;
  cout << "\tRunning file: " << fullname << endl;
  cout << "\tOutput file: " << resultfile << endl;
  //NEDeco deco(text,t,rh,modelFile,maxLabels,context,singleLabels);
  deco.Decorate(&text);
  ofstream outfile;
  outfile.open(resultfile.c_str());
  if (outfile.is_open()) {
    if (format=="SHORT")
      deco.printTRECEnts(outfile,threshold);
    else
      deco.printEnts(outfile,threshold);
    outfile.close();
  }
  else {  // if file not open, try making directory then file
    StringXML x = "mkdir " + resultsDir + " -p";
    std::system(x.c_str());
    outfile.open(resultfile.c_str());
    if (outfile.is_open()) {
      if (format=="SHORT") {
        deco.printTRECEnts(outfile,threshold);
      }
      else {
        deco.printEnts(outfile,threshold);
      }
      outfile.close();
    }
    else {
      cout << "Unable to open " << resultfile << endl;
    }
  }
  //deco.printEnts();
}

void trainFile(const StringXML& path, ostream& out,NEDeco deco,
    vector<map<StringXML,int> >& frequencies,
    vector<map<StringXML,int> >& prevFrequencies,
    const bool countFrequencies){
  StringXML text(readfile(path));
  deco.PrintTrainingData(&text,false,out,frequencies,prevFrequencies,
      countFrequencies);
}

void test(const vector<StringXML>& files,
    const vector<StringXML>& dirs,NEDeco deco,
    const StringXML& format,const StringXML resultsDir, float threshold) {
  cout << files.size() << " files, " << dirs.size() << " directories." << endl;
  cout << "Files: " << endl; 
  for (unsigned int i=0;i<files.size();i++) {
    cout << '_' << flush;
  }
  cout << endl;
  for (vector<StringXML>::const_iterator ite=files.begin();
      ite!=files.end();ite++) {
    testFile(".",*ite,deco,format,resultsDir,threshold);
    cout << '|' << flush;
  }
  cout << endl << "Directories:" << endl;
  for (vector<StringXML>::const_iterator ite=dirs.begin();
      ite!=dirs.end();ite++) {
    testDirectory(*ite,deco,format,resultsDir,threshold);
  }
}

void train(const StringXML& outputFile, const vector<StringXML>& files,
    const vector<StringXML>& dirs,NEDeco deco,const int classCount,
    const StringXML& freqFile,const StringXML& prevFreqFile) {
  ofstream out;
  out.open(outputFile.c_str());
  out << classCount << endl;
  bool countFrequencies=false;
  // check whether to count frequencies
  if (freqFile!="" || prevFreqFile!="") {
    cout << "Counting token frequencies." << endl;
    countFrequencies=true;
    if (freqFile=="") {
      cerr << "Error: output file for frequency counts not specified" << endl;
      return;
    }
    if (prevFreqFile=="") {
      cerr << "Error: output file for previous frequency counts not specified"
	   << endl;
      return;
    }
  }
  vector<map<StringXML,int> > frequencies;
  vector<map<StringXML,int> > prevFrequencies;
  for (int i=0;i<classCount;i++) {
    map<StringXML,int> m;
    map<StringXML,int> pm;
    frequencies.push_back(m);
    prevFrequencies.push_back(pm);
  }
  cout << files.size() << " files, " << dirs.size() << " directories." << endl;
  cout << "Files: " << endl;
  for (unsigned int i=0;i<files.size();i++) {
    cout << '_' << flush;
  }
  cout << endl;
  // initialise empty maps for each tag
  //for (int i=0;i<(classCount/2)-1;i++) {
  //}
  for (vector<StringXML>::const_iterator ite=files.begin();
      ite!=files.end();ite++) {
    trainFile(*ite,out,deco,frequencies,prevFrequencies,countFrequencies);
    cout << '|' << flush;
  }
  cout << endl << "Directories:" << endl;
  for (vector<StringXML>::const_iterator ite=dirs.begin();
      ite!=dirs.end();ite++) {
    trainDirectory(*ite,out,deco,frequencies,prevFrequencies,countFrequencies);
  }
  out.close();
  // dump the frequency information to file
  if (freqFile!="") {
    out.open(freqFile.c_str());
    // for each tag
    for (unsigned int j=0;j<frequencies.size();j++) {
      out << "#### Tag " << j << " --------------------" << endl;
      // print each token
      for (map<StringXML,int>::const_iterator ite=frequencies[j].begin();
          ite!=frequencies[j].end();ite++) {
        out << ite->first << " " << ite-> second << endl;
      }
      out << endl;
    }
    out.close();
  }
  if (prevFreqFile!="") {
    out.open(prevFreqFile.c_str());
    // for each tag
    for (unsigned int j=0;j<prevFrequencies.size();j++) {
      out << "#### Tag " << j << " --------------------" << endl;
      // print each token
      for (map<StringXML,int>::const_iterator ite=prevFrequencies[j].begin();
          ite!=prevFrequencies[j].end();ite++) {
        out << ite->first << " " << ite-> second << endl;
      }
      out << endl;
    }
    out.close();
  }
}

void setFeatureWeight(const StringXML& setting, FeatureHandler& fh) {
  try {
    // get the string up until the space
    istringstream is(setting);
    StringXML feature = "";
    double weight=0.0;
    is >> feature >> weight;
    if (!fh.setFeatureWeight(feature,weight)) {
      cout << "Warning: Error setting feature weight: " << setting << endl;
    }
  } catch(...) {
    cerr << "Error applying setting: " << setting << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Puts the filenames in a given directory in the given vector
////////////////////////////////////////////////////////////////////////////////
void list_directory(const StringXML& path, vector<StringXML>& flisting,
      vector<StringXML>& dirlisting) {
  fs::path dir_path(path);
  if (!fs::exists(dir_path)) {
     cerr << "Can't list directory: " << path << endl;
  }
  fs::directory_iterator end_itr; // default construction yields past-the-end
  for (fs::directory_iterator itr(dir_path); itr!=end_itr; ++itr) {
    if (fs::is_directory(*itr)) { // dir
      dirlisting.push_back(itr->leaf());
    }
    else { // file
      flisting.push_back((dir_path/itr->leaf()).leaf());
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
// Initializes the list handler, given the file that contains the list specification
////////////////////////////////////////////////////////////////////////////////
map<StringXML,EntityTag>
InitializeListHandler(const StringXML& file) {
  map<StringXML,EntityTag> listMap;
  StringXML list="";
  bool listLine=true;
  ifstream ifs;
  ifs.open(file.c_str());
  if (!ifs) {
    cout << "Error opening list specification file." << endl;
  }
  // for each line
  StringXML line="";
  while (getline(ifs,line)) {
    // if line is not a comment and is not 
    if ((line.size() > 0) && (line.substr(0,1) != "#")) {
      if (listLine) {
        list=line;
        listLine=false;
      }
      else {
        cout << "Using list: " << list << "\t" << line << endl;
        EntityTag t(line);
        listMap.insert(make_pair(list,t));
        list="";
        listLine=true;
      }
    }
  }
  ifs.close();
  return listMap;
}

////////////////////////////////////////////////////////////////////////////////
// Reads and returns the text of a file
////////////////////////////////////////////////////////////////////////////////
StringXML readfile(const StringXML& fname) {
  ifstream inFile;
  StringXML out="";
  inFile.open(fname.c_str());
  if (!inFile) {
    cout << "Unable to open file: " << fname << endl;
  }
  else {
  while (!inFile.eof()) {
    StringXML str;
    getline(inFile,str);
    out = out + "\n" + str;
    }
  }
  inFile.close();
  return out;
}
