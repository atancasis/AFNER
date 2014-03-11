///////////////////////////////////////////////////////////////////////////////
// yasmet2_large.cc
// Menno van Zaanen <menno@ics.mq.edu.au>
///////////////////////////////////////////////////////////////////////////////
// This is the regular yasmet code, but with a hugely reduced memory
// footprint.  It uses gdbm to store all events in file instead of
// memory.  Compile with:
// g++ -o yasmet2_large yasmet2_large.cc -lgdbm * -lboost_serialization
// Make sure that the dbm files are smaller than 2GB (os limit) you
// can lower the bucket_size to reduce the dbm filesize.  Make sure
// that you don't use more files than gdbm_files.
// const int bucket_size=50000;
// const int gdbm_files=400;
///////////////////////////////////////////////////////////////////////////////
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

#include <fstream> /* This program is free software; you can redistribute    */
#include <cmath>   /* it and/or modify it under the terms of the GNU General */
#include <string>  /* Public License.                                        */
#include <utility> /* Written by Franz Josef Och                             */
#include <iostream>/* This program is distributed WITHOUT ANY WARRANTY      */
#include <numeric> /* YASMET2.cc */
#include <sstream>
#include <gdbm.h>
#include <ext/hash_map>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

using namespace std; 
using namespace boost::archive;
using __gnu_cxx::hash_map;using __gnu_cxx::hash;


template<class _Key, class _Tp>
class cached_hash_map {
public:
  cached_hash_map() {
    _dbf = gdbm_open("yasmet_hash_map.dbm", 0, GDBM_NEWDB|GDBM_FAST, 0666, 0);
    if (!_dbf) {
      cerr <<  "error opening yasmet_hash_map.dbm" << endl;
      exit (2);
    }
  }

  ~cached_hash_map() {
    gdbm_close(_dbf);
  }

  _Tp
  read(const _Key& key) {
    datum db_key;
    datum db_content;
    db_key.dsize=sizeof(key);
    db_key.dptr=(char*)&key;
    db_content=gdbm_fetch(_dbf, db_key);
    if (db_content.dsize>0) {
      _Tp ret(*(_Tp*)db_content.dptr);
      free(db_content.dptr);
      return ret;
    } else {
      cerr << "FATAL: Key not found." << endl;
      exit(-3);
    }
  }

  void
  write(const _Key& key, const _Tp& value) {
    datum db_key;
    datum db_content;
    db_key.dsize=sizeof(key);
    db_key.dptr=(char*)&key;
    db_content.dsize=sizeof(value);
    db_content.dptr=(char*)&value;
    int ret=gdbm_store(_dbf, db_key, db_content, GDBM_INSERT);
    if (ret==-1) {
      cerr << "write error." << endl;
      cerr << gdbm_strerror(gdbm_errno) << endl;
    }
  }

  bool
  count(const _Key& key) const {
    datum db_key;
    db_key.dsize=sizeof(key);
    db_key.dptr=(char*)&key;
    bool ret=gdbm_exists(_dbf, db_key);
    return ret?1:0;
  }

private:
  bool _open;
  GDBM_FILE _dbf;
};

const int bucket_size=15000;
const int gdbm_files=600;

template<class _Tp>
class cached_vector {
public:
  cached_vector():_counter(0) {
    for (int i=0; i!=gdbm_files; ++i) {
      _files_used[i]=false;
    }
  }

  ~cached_vector() {
    for (int i=0; i!=gdbm_files; ++i) {
      if (_files_used[i]) {
        gdbm_close(_dbfv[i]);
      }
    }
  }

  _Tp
  read(typename vector<_Tp>::size_type key) {
    int index=key/bucket_size;
    if (index>=gdbm_files) {
      cerr << "array out of bounds" << endl;
      exit(-3);
    }
    if (!_files_used[index]) {
      cerr << "FATAL: Key not found." << endl;
      exit(-3);
    }
    datum db_key;
    datum db_content;
    db_key.dsize=sizeof(key);
    db_key.dptr=(char*)&key;
    db_content = gdbm_fetch(_dbfv[index], db_key);
    if (db_content.dptr!=NULL) {
      string cont(db_content.dptr, db_content.dsize);
      free(db_content.dptr);
      stringstream ss(cont);
      binary_iarchive ti(ss);
      _Tp ret;
      ti >> BOOST_SERIALIZATION_NVP(ret);
      return ret;
    } else {
      cerr << "FATAL: Key not found." << endl;
      exit(-3);
    }
  }

  void 
  push_back(const _Tp& value) {
    write(_counter++, value);
  }

  void
  write(const typename vector<_Tp>::size_type& key, const _Tp& value) {
    int index=key/bucket_size;
    if (index>=gdbm_files) {
      cerr << "array out of bounds" << endl;
      exit(-3);
    }
    if (!_files_used[index]) {
      stringstream file;
      file << "yasmet_vector" << index << ".dbm";
      char *fn=(char*)malloc(strlen(file.str().c_str()));
      strcpy(fn, file.str().c_str());
      _dbfv[index]=gdbm_open(fn, 0, GDBM_NEWDB|GDBM_FAST, 0666, 0);
      free(fn);
      _files_used[index]=true;
    }
    if (!_dbfv[index]) {
      cerr << "FATAL: couldn't open file" << endl;
      exit(2);
    }
    datum db_key;
    datum db_content;
    db_key.dsize=sizeof(key);
    db_key.dptr=(char*)&key;
    stringstream ss;
    binary_oarchive to(ss);
    to << BOOST_SERIALIZATION_NVP(value);
    string cont=ss.str();
    db_content.dsize=cont.size()+1;
    db_content.dptr=(char*)cont.c_str();
    int ret=gdbm_store(_dbfv[index], db_key, db_content, GDBM_REPLACE);
    if (ret!=0) {
      cerr << "write error." << endl;
      cerr << gdbm_strerror(gdbm_errno) << endl;
    }
  }

  int
  size() const {
    return _counter;
  }

private:

  cached_vector(const cached_vector<_Tp>&) { throw this; }

  cached_vector&
  operator=(const cached_vector<_Tp>&) { throw this; }

  GDBM_FILE _dbfv[gdbm_files];
  bool _files_used[gdbm_files];
  int _counter;

};

typedef double D;
typedef pair<int,D> fea;
typedef vector<fea> vfea;
size_t v=0;
template<class T> ostream &operator<<(ostream&out,const vector<T>&x){
  copy(x.begin(),x.end(),ostream_iterator<T>(out," "));return out<<endl;}
struct Z{D k,q,l;Z(D a=0,D b=-1,D c=0):k(a),q(c),l((b<0)?1:log(b)){};};
struct expclass{D operator()(D x){return exp(x);}};
ostream&operator<<(ostream&o,const Z&x){return o<<x.k<<","<<x.l<<','<<x.q<<' ';}
struct hash_str{size_t operator()(const string&s)const
  {return hash<const char*>()(s.c_str());}};
struct event {
  vector<D> Ny;
  size_t y;
  vector<vfea> f;
  vector<D> fs;

  vector<D>&
  computeProb(const vector<Z>&z,vector<D>&pr)const {
    vector<D>::iterator p=pr.begin(),pb=pr.begin(),pe=pr.end();
    for(vector<vfea >::const_iterator fi=f.begin();fi!=f.end();++fi,++p){*p=0.0;
      for(vfea::const_iterator j=fi->begin();j!=fi->end();++j)
  *p+=z[j->first].l*j->second;}
    transform(pb,pe,pb,bind2nd(plus<D>(),-*max_element(pb,pe)));
    transform(pb,pe,pb,expclass());
    transform(pb,pe,pb,bind2nd(divides<D>(),accumulate(pb,pe,0.0)));
    return pr;
  }

	template<class Archive> void
  serialize(Archive & ar, const unsigned int version) {
    ar & y;
    ar & Ny;
    ar & f;
    ar & fs;
  }

};
int main(int argc,char**av){string s; int unseenFeaturesZero=0;
  cached_vector<pair<event,double> >E;bool lN=0;D TRN=0.0,TST=0.0;bool qt=0,initmu=0;
  vector<pair<string,D> > s2f;vector<Z> z;ifstream *muf=0;int mfc=-2,kfl=0;
  size_t st=0,C=0,it=0,maxIt=1000,ts=0,noF=0,Es,I,N=100000000;D ssi=0.0;
  D d,l=1e30,old_l,w=0.0,dSmoothN=0.0,minBetter=0.01,lt=0.0,wt=0.0,F=0.0;
  for(int i=1;i<argc;++i){string si(av[i]);
    if(si=="-v"||si=="-V")v=1+(si=="-V"); else if(si=="-q")qt=1;
    else if(si=="-red") mfc=atoi(av[++i]);else if(si=="-initmu") initmu=1;
    else if(si=="-lNorm") lN=1;   else if(si=="-iter")maxIt=atoi(av[++i]);          
    else if(si=="-dN") dSmoothN=atof(av[++i]);
    else if(si=="-deltaPP") minBetter=atof(av[++i]);
    else if(si=="-smooth") ssi=1.0/pow(atof(av[++i]),2);
    else if(si=="-kw"||si=="-kr") kfl=1+(si=="-kr");
    else if(si=="-unseenFeaturesZero") unseenFeaturesZero=1;
    else if(av[i][0]=='-'){
      cerr << "\nUsage: " << av[0] << "[-v|-V|-red n|-iter n|-dN d|-lNorm"
  "|-deltaPP dpp][mu]\n none: GIS \n -red: count-based feature sel"
  "ection\n   mu: test pp\n-iter: number of iterations\n  -dN: smoothing "
  "of event counts\n-lNorm: length normalization\n-deltaPP: end criterion"
  "minimal change of perplexity\n-kw: write K file\n-kr: read K file\n";
      return 0;}else muf=new ifstream(av[i]);}
  {hash_map<string,int,hash_str> f2s;event e;size_t curY=0;double wi=1.0;
  s2f.push_back(pair<string,D>("@@@CORRECTIVE-FEATURE@@@",0.0));
  z.push_back(Z());f2s["@@@CORRECTIVE-FEATURE@@@"]=0;
  if(muf){
    while(*muf>>s>>d){size_t p=(!f2s.count(s))?(f2s[s]=s2f.size()):(f2s[s]);
      Z k(0,kfl?1:d,0);pair<string,D> sd(s,0.0);
      if(p<s2f.size())s2f[p]=sd;else s2f.push_back(sd);
      if(p<z.size())z[p]=k;else z.push_back(k);}}
  int line=0;
  while(cin>>s)switch(st){
    case 0: C=atoi(s.c_str());st=1;break;
    case 1: cerr << line++ << endl; if(s=="TEST"){N=E.size();}else{e.f.resize(C);e.fs.resize(C);
      e.y=atoi(s.c_str());curY=0;st=2;wi=1.0;e.Ny.resize(C);
      for(size_t c=0;c<C;++c)e.Ny[c]=(c==e.y)?(1.0-dSmoothN):(dSmoothN/(C-1));}
      break;
    case 2:if(s=="#")st=3;else if(s=="@")st=4;else if(s=="$")st=5;else abort();
      break;
    case 3: case 4:
      if(s=="#") {if( ++curY==C ) {
  st=1;{E.push_back(make_pair(e,wi));e=event();}

  if(v==2){cerr<<"E:"<<E.size()<< " "<<s2f.size()<<"  \r";cerr.flush();}}}
      else {D fc=1.0;if(st==4){string t;cin>>t;fc=atof(t.c_str());}
        if(st==4&&s=="@"){e.Ny[curY]=fc;}
  else{
    if(kfl!=1||(e.Ny[curY]!=0)){
      if(!f2s.count(s)){
        if((muf&&kfl==0)||E.size()>=N||kfl==2)
    {if(v>1)cerr<<"new "<<s<<" (igd)"<<endl;break;}
        else {f2s[s]=s2f.size();s2f.push_back(make_pair(s,0.0));}}
      if(E.size()<N)s2f[f2s[s]].second+=(e.Ny[curY]!=0);
      e.f[curY].push_back(make_pair(f2s[s],fc));
      e.fs[curY]+=fc;}
  if(E.size()<N)F=max(F,e.fs[curY]);}}break;
    case 5:wi=atof(s.c_str());st=2;break;}
  Es=E.size();ts=max(int(Es-N),0);N=Es-ts;I=s2f.size();
  cerr<<"I: "<<I<<" F: "<<F<<endl;}
  vector<D> p(C);z.resize(I);   if(initmu==0&&muf&&kfl!=2){N=0;ts=E.size();}
  for(size_t n=0;n<Es;++n){
    pair<event,double> ped=E.read(n);
    event&en=ped.first;((n<N)?TRN:TST)+=ped.second;
    for(size_t c=0;c<C;++c){
      if(lN)for(size_t k=0;k<en.f[c].size();++k)en.f[c][k].second/=en.fs[c];
      else en.f[c].push_back(make_pair(0,F-en.fs[c]));} E.write(n, ped); }
  if(lN)F=1.0;
  if( mfc!= -2){cout << C;
  for(size_t n=0;n<Es;++n){pair<event,double> ped=E.read(n); const event&en=ped.first;
    if(n==N)cout<<"\nTEST";
    cout<<endl<<en.y<<" $ "<< ped.second<<" @ ";
    for(size_t i=0;i<C;++i){cout << "@ " << en.Ny[i] << " ";
    for(size_t j=0;j<en.f[i].size();++j)if(s2f[en.f[i][j].first].second>mfc)
      cout<<s2f[en.f[i][j].first].first<<" "<<en.f[i][j].second<<" " ;
    cout<<"# ";}}cout<<endl;}
  else {for(size_t i=0;i<N;++i){pair<event,double> ped=E.read(i); const event&ei=ped.first;D wi=ped.second;
    for(size_t y=0;y<ei.Ny.size();++y)for(size_t j=0;j<ei.f[y].size();++j)
      z[ei.f[y][j].first].k+=wi*ei.Ny[y]*ei.f[y][j].second;}
    if(kfl==1){for(size_t i=0;i<s2f.size();++i)
      {cout<<s2f[i].first<<" "<<z[i].k <<endl;}exit(0);}
    for(size_t i=0;i<I;++i)if(!z[i].k)noF++;
    if(noF&&N)cerr<<"I': "<<I-noF<<endl;
    if(v>1)cerr << "Expected feature counts: " << z;
    do{old_l=l;l=0.0;w=0.0;lt=0.0;wt=0.0;double wx=0.0,wtx=0.0,lx=0,ltx=0;
      for(size_t i=0;i<I;++i)z[i].q=0;
      for(size_t i=0;i<Es;++i){pair<event,double> ped=E.read(i); const event&ei=ped.first;double wi=ped.second;
      ei.computeProb(z,p);
      vector<D>::const_iterator me=max_element(p.begin(),p.end());
      if(!N){cout<<me-p.begin() << " " << p;}
      if(i<N){
  for(size_t j=0;j<C;++j){const vfea&eifj=ei.f[j];D pj=p[j]*wi;
  for(vfea::const_iterator k=eifj.begin();k!=eifj.end();++k)
    z[k->first].q+=pj*k->second;}}
      ((i<N)?l:lt)-=wi*log(p[ei.y]);
      for(size_t y=0;y<C;++y)((i<N)?lx:ltx)-=ei.Ny[y]*wi*log(p[y]);
      ((i<N)?w:wt)+=wi*((p.begin()+ei.y)!=me);
      ((i<N)?wx:wtx)+=wi*(ei.Ny[me-p.begin()]==0.0&&(p.begin()+ei.y)!=me);}
    for(size_t i=0;i<I;++i){Z&x=z[i];
      if(x.k){double dl=(log(x.k)-log(x.q))/F,ddl=1.0;
        if(ssi!=0.0)for(int iter=0;iter<20&&ddl>1e-10;++iter,dl-=ddl)
    {double h=x.q*exp(dl*F);ddl=(h+(x.l+dl)*ssi-x.k)/(h*F+ssi);}
  x.l+=dl;}
      else if(unseenFeaturesZero)x.l= -100;}
    if(v>1)cerr<<it<< ". "<<" KLQ:"<<z<< " " << p<<"\n";
    if(TRN&&!qt)cerr<<it<<". "<<"pp: "<<exp(l/TRN)<<" er: "<<w/TRN<<" erx: " 
        << wx/TRN << " ppx: " << exp(lx/TRN);if(!TST)cerr<<endl;
    if(TST)cerr<<" "<<"tst-pp: "<<exp(lt/TST)<<" tst-er: "<<wt/TST
         <<" tst-erx: "<< wtx/TST<<" tst-ppx: "<<exp(ltx/TST)<<endl;
    } while(fabs(exp(l/TRN)-exp(old_l/TRN))>minBetter&&it++<maxIt&&N);
    if(N)for(size_t i=0;i<I;++i)
      cout<<s2f[i].first<<" "<<exp(z[i].l-z[0].l)<<'\n';}}

