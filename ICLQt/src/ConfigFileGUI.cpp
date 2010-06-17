/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ConfigFileGUI.cpp                            **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQt/ConfigFileGUI.h>
#include <ICLUtils/StringUtils.h>

#include <QTreeWidgetItem>

#include <ICLQt/ButtonHandle.h>
#include <ICLQt/BoxHandle.h>
#include <ICLQt/SplitterHandle.h>
#include <ICLQt/SliderHandle.h>
#include <ICLQt/ComboHandle.h>
#include <ICLQt/FSliderHandle.h>

#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>


#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>

#include <ICLUtils/Size.h>
#include <ICLUtils/Size32f.h>

#include <ICLUtils/Range.h>

#include <ICLUtils/Rect.h>

using std::string;

namespace icl{

  static QString get_entry_type(const ConfigFile::Entry &e){
    return e.getTypeName().c_str();
  }

  
  typedef QString (*entry_to_string_func)(const std::string &key,const ConfigFile &cfg);

  QString entry_to_string_no_op(const std::string&,const ConfigFile&){
    return "---";
  }

  
  static QString get_entry_text(const ConfigFile::Entry &e){
    return e.value.c_str();
  }



  QTreeWidgetItem *find_child(QTreeWidgetItem *parent, const char *name){
    for(int i=0;;++i){
      QTreeWidgetItem *ci = parent->child(i);
      if(ci){
        if(ci->text(0) == name) return ci;
      }else{
        return 0;
      }
    }
    return 0;
  }

  void ConfigFileGUI::saveAs(){
    QString fnNew = QFileDialog::getSaveFileName(m_tree,"save...","./","XML-Files (*.xml)");
    if(fnNew == "") return;
    m_config->save(fnNew.toLatin1().data());
  }

  void ConfigFileGUI::load(){
    QString fn = QFileDialog::getOpenFileName(m_tree,"load ...","./","XML-Files (*.xml)");
    if(fn != ""){
      ConfigFile *newCfg = new ConfigFile(fn.toLatin1().data());
      if(m_own) delete m_config;
      m_own = true;
      m_config = newCfg;
      loadConfig(*m_config);
    }
  }

  ConfigFileGUI::~ConfigFileGUI(){
    delete m_tree;
  }
  void ConfigFileGUI::updateTree(){
    loadConfig(*m_config);
  }
  
  ConfigFileGUI::ConfigFileGUI(const ConfigFile &config, QWidget *parent) throw(ICLException):
    GUI("vbox[@handle=parent]",parent),m_own(false),m_tree(new QTreeWidget){
    
    m_tree->setColumnCount(3);
    m_tree->setColumnWidth(0,250);
    m_tree->setColumnWidth(1,200);
    m_tree->setColumnWidth(2,50);

    loadConfig(config);
    

    GUI buttons("hbox[@handle=right@maxsize=1000x2]");
    buttons << "button(collapse)[@handle=collapse]";
    buttons << "button(expand)[@handle=expand]";
    buttons << "button(save)[@handle=save-as]";
    buttons << "button(load)[@handle=load]";
    buttons << "button(update)[@handle=update]";

    (*this) << buttons;
    (*this) << str("vbox[@handle=left@label=config:")+config.get<std::string>("config.title","no title")+"]";
    
    this->create();
    
    connect(*getValue<ButtonHandle>("collapse"),SIGNAL(clicked()),m_tree,SLOT(collapseAll()));
    connect(*getValue<ButtonHandle>("expand"),SIGNAL(clicked()),m_tree,SLOT(expandAll()));
    //connect(*getValue<ButtonHandle>("save"),SIGNAL(clicked()),this,SLOT(save()));
    connect(*getValue<ButtonHandle>("save-as"),SIGNAL(clicked()),this,SLOT(saveAs()));
    connect(*getValue<ButtonHandle>("load"),SIGNAL(clicked()),this,SLOT(load()));
    //connect(*getValue<ButtonHandle>("reload"),SIGNAL(clicked()),this,SLOT(reload()));
    connect(*getValue<ButtonHandle>("update"),SIGNAL(clicked()),this,SLOT(updateTree()));
    
    (*getValue<BoxHandle>("parent"))->resize(500,500);

    getValue<BoxHandle>("left").add(m_tree);
    connect(m_tree,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this,SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));

    // now we need components for these data types
    /*
      - char 
      - unsigned char 
      - short 
      - unsigned short 
      - int 
      - unsigned int
      - float 
      - double 
      - string 
      - Size 
      - Point 
      - Rect 
      - Range32s
      - Range32f
      - bool
    */
  }

  QWidget *ConfigFileGUI::getWidget(){
    return *getValue<BoxHandle>("parent");
  }

  void ConfigFileGUI::loadConfig(const ConfigFile &config){
    m_config = const_cast<ConfigFile*>(&config);
    m_tree->clear();

    QTreeWidgetItem *root = new QTreeWidgetItem(m_tree);
    QStringList header; header << "entry" << "value" << "type";
    m_tree->setHeaderLabels (header);
    root->setText(0,"config");
    m_tree->insertTopLevelItem(0,root);
    const std::vector<const ConfigFile::Entry*> es = config.getEntryList();
    for(unsigned int i=0;i<es.size();++i){
      if(es[i]->id == "config.config.title") continue; // some error here -> todo fix in config file
      if(es[i]->id == "config.title") continue; // some error here -> todo fix in config file
      const std::vector<std::string> tk = tok(es[i]->id,".");
      ICLASSERT_RETURN(tk.front() == "config");
      QTreeWidgetItem *item = root;
      for(unsigned int j=1;j<tk.size();++j){
        QTreeWidgetItem *c = find_child(item,tk[j].c_str());
        if(c){
          if(j<tk.size()-1){
            item = c;
          }else{
            throw ICLException(str("entry found twice: ")+es[i]->id);
          }
        }else{
          if(j<tk.size()-1){
            QTreeWidgetItem *n = new QTreeWidgetItem(item);
            n->setText(0,tk[j].c_str());
            item->addChild(n);
            item = n;
          }else{
            QTreeWidgetItem *n = new QTreeWidgetItem(item);
            QString t = get_entry_type(*es[i]);
            QString e = get_entry_text(*es[i]);
            n->setForeground(0,QColor(60,150,255));
            n->setText(0,tk[j].c_str());
            n->setText(1,e);
            n->setText(2,t);
            item->addChild(n);
#define YES_USE_SLIDERS_PLEASE
#ifdef YES_USE_SLIDERS_PLEASE
            if(t == "string"){
              const ConfigFile::KeyRestriction *restriction = es[i]->restr.get();
              if(restriction && restriction->hasValues){
                const std::string &values = restriction->values;
                std::string p = str("[@minsize=3x1@out=v@handle=h]");
                m_guis.push_back(NamedGUI());
                GUI &gui = m_guis.back().gui;
                gui = GUI(str("combo(")+values+")"+p);
                gui.create();
                
                ComboHandle &ch = gui.getValue<ComboHandle>("h");
                ch.registerCallback(SmartPtr<GUI::Callback>(this,false));
                
                int idx = (*ch)->findText(e);
                if(idx!=-1){
                  (*ch)->setCurrentIndex(idx);
                }else{
                  ERROR_LOG("Entry: " << es[i]->id << ":\nValue list does not contain initial value");
                }
                
                m_guis.back().id = es[i]->getRelID();
                m_guis.back().type = t.toLatin1().data();
                m_guis.back().item = n;
                m_tree->setItemWidget(n,1,*gui.getValue<ComboHandle>("h"));

                n->setText(1,"");
              }
            }else if(t == "bool"){
              m_guis.push_back(NamedGUI());
              GUI &gui = m_guis.back().gui;
              bool bo = icl::parse<bool>(e.toLatin1().data());
              std::string tr = bo ? "!true" : "true";
              gui = GUI("togglebutton(false,"+tr+")[@handle=b@out=v@minsize=5x1]");
              gui.create();
              gui.getValue<ButtonHandle>("b").registerCallback(SmartPtr<GUI::Callback>(this,false));
              m_guis.back().id = es[i]->getRelID();
              m_guis.back().type = t.toLatin1().data();
              m_guis.back().item = n;
              m_tree->setItemWidget(n,1,*gui.getValue<ButtonHandle>("b"));
              n->setText(1,"");
            }else if(t == "float" || t == "int"){
              const ConfigFile::KeyRestriction *restriction = es[i]->restr.get();
              if(restriction && restriction->hasRange){
                const Range64f r(restriction->min,restriction->max);
                std::string p = str("[@minsize=5x1@out=v@handle=h]");
                m_guis.push_back(NamedGUI());
                GUI &gui = m_guis.back().gui;
                gui = GUI("hbox[@handle=b]");
                std::string el = e.toLatin1().data();
                bool ok = true;
                if(!r.contains(to64f(el))){
                  ERROR_LOG("Entry: " << es[i]->id << ":\nInitial value is out of given range");
                }
                if(t == "float"){
                  gui << str("fslider(")+str(r.minVal)+','+str(r.maxVal)+','+el+')'+p;
                  gui.create();
                  gui.getValue<FSliderHandle>("h").registerCallback(SmartPtr<GUI::Callback>(this,false));
                }else if (t == "int"){
                  gui << str("slider(")+str((int)r.minVal)+','+str((int)r.maxVal)+','+el+')'+p;
                  gui.create();
                  gui.getValue<SliderHandle>("h").registerCallback(SmartPtr<GUI::Callback>(this,false));
                }else{
                  ok = false;
                }
                
                if(ok){

                  m_guis.back().id = es[i]->getRelID();
                  m_guis.back().type = t.toLatin1().data();
                  m_guis.back().item = n;
                  m_tree->setItemWidget(n,1,*gui.getValue<BoxHandle>("b"));
                  n->setText(1,"");
                }else{
                  m_guis.pop_back();
                }
              }
            }
#endif
          }
        }
      }
    }
    m_tree->expandToDepth(2);
 
  }
  
  static QString get_syntax(const QString &type){
    std::map<QString,QString> ts;
    if(!ts.size()){
      ts["char"] = "<character>";
      ts["unsigned char"] = "<8-Bit positive Integer> {0,1,..,255}";
      ts["int"] = "<signed integer-value>";
      ts["unsigned int"] = "<positive integer-value>";
      ts["char"] = "<character>";
      ts["short"] = "<16-Bit signed Integer> {-32768,..,32767}";
      ts["unsigned char"] = "<16-Bit positive Integer> {0,..,65535}";
      ts["float"] = "<single precision floating point number>";
      ts["double"] = "<double precision floating point number>";
      ts["string"] = "<any text entry>";
      ts["Size"] = "<WIDTHxHEIGHT> or fixed (VGA,SVGA,...)";
      ts["Point"] = "<(X,Y)> (X and Y are signed integers";
      ts["Rect"] = "<WxH@(X,Y)>";
      ts["Range32s"] = "<[MIN,MAX]> (MIN and MAX are signed integers)";
      ts["Range32f"] = "<[MIN,MAX]> (MIN and MAX are floats)";
    }
    std::map<QString,QString>::const_iterator it = ts.find(type);
    if(it != ts.end()){
      return it->second;
    }else{
      return "<no syntax definition available>";
    }
  }

  
  typedef bool (*update_entry_func)(const std::string&,const std::string &,ConfigFile &cfg);
  
  template <class T>
  bool update_entry_templ(const std::string &key,const std::string &val,ConfigFile &cfg){
    try{
      cfg.set<T>(key,parse<T>(val));
    }catch(ICLException &ex){
      return false;
    }
    return true;
  }

  template<> bool update_entry_templ<char>(const std::string &key,const std::string &val,ConfigFile &cfg){
    if(val.size() == 1){
      cfg.set<char>(key,val[0]);
      return true;
    }else {
      return false;
    }
  }
  template<> bool update_entry_templ<unsigned char>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    int i = QString(val.c_str()).toInt(&ok);
    if(ok && i>= 0 && i<=255){
      cfg.set<unsigned char>(key,i);
      return true;
    }else{
      return false;
    }
  }
  template<> bool update_entry_templ<int>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    int i = QString(val.c_str()).toInt(&ok);
    if(ok){
      cfg.set<int>(key,i);
      return true;
    }else{
      return false;
    }
  }
  template<> bool update_entry_templ<unsigned int>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    unsigned int i = QString(val.c_str()).toUInt(&ok);
    if(ok){
      cfg.set<unsigned int>(key,i);
      return true;
    }else{
      return false;
    }
  }
  template<> bool update_entry_templ<short>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    short i = QString(val.c_str()).toShort(&ok);
    if(ok){
      cfg.set<short>(key,i);
      return true;
    }else{
      return false;
    }
  }
  template<> bool update_entry_templ<unsigned short>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    short i = QString(val.c_str()).toUShort(&ok);
    if(ok){
      cfg.set<unsigned short>(key,i);
      return true;
    }else{
      return false;
    }
  }
  template<> bool update_entry_templ<float>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    float f = QString(val.c_str()).toFloat(&ok);
    if(ok){
      cfg.set<float>(key,f);
      return true;
    }else{
      return false;
    }
  }
  template<> bool update_entry_templ<double>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    float f = QString(val.c_str()).toDouble(&ok);
    if(ok){
      cfg.set<double>(key,f);
      return true;
    }else{
      return false;
    }
  }

  template<> bool update_entry_templ<std::string>(const std::string &key,const std::string &val,ConfigFile &cfg){
    cfg.set<std::string>(key,val);
    return true;
  }

  Size toSize(const QString &val, bool *ok){
    Size s(val.toLatin1().data());
    if(s != Size(-1,-1)){
      *ok = true;
      return s;
    }
    int w = val.section('x',0,0).toInt(ok);
    if(!*ok) return Size::null;
    int h = val.section('x',1,1).toInt(ok);
    if(!*ok) return Size::null;
    return Size(w,h);
  }
  
  template<> bool update_entry_templ<Size>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    Size s = toSize(val.c_str(),&ok);
    if(ok){
      cfg.set<Size>(key,s);
      return true;
    }else{
      return false;
    }
  }
  
  Point toPoint(QString val, bool *ok){
    if(val.length() < 5){
      *ok = false;
      return Point::null;
    }
    if(val[0] != '(' || val[val.length()-1] != ')'){
      *ok = false;
      return Point::null;
    }
    val = val.mid(1,val.length()-2);
    int x = val.section(',',0,0).toInt(ok);
    if(!*ok) return Point::null;
    int y = val.section(',',1,1).toInt(ok);
    if(!*ok) return Point::null;
    return Point(x,y);
  }

  template<> bool update_entry_templ<Point>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    Point p = toPoint(val.c_str(),&ok);
    if(ok){
      cfg.set<Point>(key,p);
      return true;
    }else{
      return false;
    }
  }
  template<> bool update_entry_templ<Rect>(const std::string &key,const std::string &val,ConfigFile &cfg){
    QString qk = val.c_str();
    bool ok = false;
    Size s = toSize(qk.section(')',1,1),&ok);
    if(!ok) return false;
    Point p = toPoint(qk.section(')',0,0).mid(1),&ok);
    if(ok){
      cfg.set<Rect>(key,Rect(p,s));
      return true;
    }else{
      return false;
    }
  }

  Range32s toRange32s(QString val, bool *ok){
    if(val.length() < 5){
      *ok = false;
      return Range32s();
    }
    if(val[0] != '[' || val[val.length()-1] != ']'){
      *ok = false;
      return Range32s();
    }
    val = val.mid(1,val.length()-2);
    int minVal = val.section(',',0,0).toInt(ok);
    if(!*ok) return Range32s();
    int maxVal = val.section(',',1,1).toInt(ok);
    if(!*ok) return Range32s();
    return Range32s(minVal,maxVal);
  }
  
  Range32f toRange32f(QString val, bool *ok){
    if(val.length() < 5){
      *ok = false;
      return Range32f();
    }
    if(val[0] != '[' || val[val.length()-1] != ']'){
      *ok = false;
      return Range32f();
    }
    val = val.mid(1,val.length()-2);
    float minVal = val.section(',',0,0).toFloat(ok);
    if(!*ok) return Range32f();
    float maxVal = val.section(',',1,1).toFloat(ok);
    if(!*ok) return Range32f();
    return Range32f(minVal,maxVal);
  }
  
  template<> bool update_entry_templ<Range32s>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    Range32s r = toRange32s(val.c_str(),&ok);
    if(ok){
      cfg.set<Range32s>(key,r);
      return true;
    }else{
      return false;
    }
  }

  template<> bool update_entry_templ<Range32f>(const std::string &key,const std::string &val,ConfigFile &cfg){
    bool ok = false;
    Range32f r = toRange32f(val.c_str(),&ok);
    if(ok){
      cfg.set<Range32f>(key,r);
      return true;
    }else{
      return false;
    }
  }

  
  static bool update_config_entry(const std::string &key,
                                  const std::string &val, 
                                  const std::string &type, 
                                  const ConfigFile &cfg){
    static std::map<std::string,update_entry_func> ue;
    if(!ue.size()){
#define E(T) ue[#T] = update_entry_templ<T>;
      E(char);E(unsigned char);E(short);E(unsigned short);E(int);
      E(unsigned int);E(float);E(double);E(string);E(Size);
      E(Point);E(Rect);E(Range32s);E(Range32f);
#undef E
    }
    std::map<std::string,update_entry_func>::iterator it = ue.find(type);
    if(it != ue.end()){
      cfg.lock();
      bool ok = it->second(key,val,const_cast<ConfigFile&>(cfg));
      cfg.unlock();
      return ok;

    }else{
      ERROR_LOG("undefined type:" << type);
      return false;
    }
  }

  void ConfigFileGUI::exec(const std::string &handle){
    // not used yet ... 
    // here is space for a later optimization ...
  }

  void ConfigFileGUI::exec(){
    for(std::list<ConfigFileGUI::NamedGUI>::iterator it = m_guis.begin();it!=m_guis.end();++it){
      if(it->type == "int"){
        int i = it->gui.getValue<int>("v");
        m_config->set(it->id,i);
      }else if(it->type == "float"){
        float f = it->gui.getValue<float>("v");
        m_config->set(it->id,f);
      }else if(it->type == "string"){
        std::string s = it->gui.getValue<std::string>("v");
        m_config->set(it->id,s);
      }else if(it->type == "bool"){
        bool s = it->gui.getValue<bool>("v");
        m_config->set(it->id,s);
      }
    }
  }

  void ConfigFileGUI::itemDoubleClicked(QTreeWidgetItem *item, int column){
    if(column != 1 || item->text(2) == "unsupported" || item->text(2) == ""){
      return;
    }
    for(std::list<ConfigFileGUI::NamedGUI>::iterator it = m_guis.begin();it!=m_guis.end();++it){
      if(it->item == item){
        return;
      }
    }
    QStringList keys;
    for(QTreeWidgetItem *curr = item; curr ;curr=curr->parent()){
      keys.push_front(curr->text(0));
    }
    QString key = keys.join(".");
    if(m_config->getPrefix().length()){
      key = std::string(key.toLatin1().data()).substr(m_config->getPrefix().length()).c_str();
    }
    //std::cout << "key was :" << key.toLatin1().data() << std::endl;
    
    QStringList dialogText;
    dialogText << "Adjust the value of"
               << item->text(0)
               << "Syntax:"
               << get_syntax(item->text(2));
    
    
    
    bool ok = false;
    QString t = QInputDialog::getText(m_tree,"Change config value",dialogText.join("\n"),QLineEdit::Normal,
                                      item->text(1),&ok);
    if(!ok) return;
    ok = update_config_entry(key.toLatin1().data(),t.toLatin1().data(),item->text(2).toLatin1().data(),*m_config);
    if(ok){
      item->setText(1,t);
    }else{
      QMessageBox::warning(m_tree,"Warning...","Syntax was not correct!\nNew values is ignored.");
    }
  }
  
}
