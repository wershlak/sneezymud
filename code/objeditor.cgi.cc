#include "stdsneezy.h"
#include "database.h"
#include "session.cgi.h"

#include <vector>
#include <map>
#include <list>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTTPPlainHeader.h"
#include "cgicc/HTMLClasses.h"
#include <cgicc/HTTPCookie.h>
#include <cgicc/CgiEnvironment.h>
#include <cgicc/HTTPRedirectHeader.h>

#include <sys/types.h>
#include <md5.h>

using namespace cgicc;


void sendJavaScript();
sstring mudColorToHTML(sstring);

void sendObjlist(int);
void sendShowObj(int, int, bool);
void sendShowExtra(int, int);
void saveObj(Cgicc, int);
void saveExtra(Cgicc, int);
void makeNewObj(Cgicc, int, bool);
void makeNewExtra(Cgicc, int);
void delExtra(Cgicc, int);
void delObj(Cgicc, int);

bool checkPlayerName(int account_id, sstring name)
{
  TDatabase db(DB_SNEEZY);

  db.query("select 1 from player where lower(name)=lower('%s') and account_id=%i", name.c_str(), account_id);

  if(db.fetchRow())
    return true;
  return false;
}

sstring getPlayerNames(int account_id)
{
  TDatabase db(DB_SNEEZY);
  sstring names;

  db.query("select lower(name) as name from player where account_id=%i",
	   account_id);

  if(db.fetchRow())
    names=fmt("'%s'") % db["name"];

  while(db.fetchRow()){
    names+=fmt(", '%s'") % db["name"];
  }

  return names;
}


int main(int argc, char **argv)
{
  // trick the DB code into use prod database
  gamePort=PROD_GAMEPORT;

  Cgicc cgi;
  form_iterator state_form=cgi.getElement("state");
  TSession session(cgi, "SneezyMUD");

  if(!session.isValid()){
    session.doLogin(cgi, "objeditor.cgi");
    return 0;
  }

  if(!session.hasWizPower(POWER_BUILDER)){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("Object Load Logs")) << endl;
    cout << body() << endl;
    cout << "You don't have permission to use this.";
    cout << body() << endl;
    return 0;
  }

    

  if(state_form == cgi.getElements().end() || **state_form == "main"){
    sendObjlist(session.getAccountID());
    return 0;
  } else if(**state_form == "delobj"){
    delObj(cgi, session.getAccountID());
    sendObjlist(session.getAccountID());
    return 0;    
  } else if(**state_form == "newobj"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Objeditor") << endl;
    cout << head() << body() << endl;

    makeNewObj(cgi, session.getAccountID(), session.hasWizPower(POWER_LOAD));
    sendShowObj(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;    
  } else if(**state_form == "showobj"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Objeditor") << endl;
    cout << head() << body() << endl;
    
    sendShowObj(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;
  } else if(**state_form == "delextra"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Objeditor") << endl;
    cout << head() << body() << endl;

    delExtra(cgi, session.getAccountID());
    sendShowExtra(session.getAccountID(), convertTo<int>(**vnum));
    return 0;    
  } else if(**state_form == "newextra"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Objeditor") << endl;
    cout << head() << body() << endl;

    makeNewExtra(cgi, session.getAccountID());
    sendShowExtra(session.getAccountID(), convertTo<int>(**vnum));
    return 0;    
  } else if(**state_form == "showextra"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Objeditor") << endl;
    cout << head() << body() << endl;
    
    sendShowExtra(session.getAccountID(), convertTo<int>(**vnum));
    return 0;
  } else if(**state_form == "saveextra"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Objeditor") << endl;
    cout << head() << body() << endl;

    saveExtra(cgi, session.getAccountID());
    sendShowExtra(session.getAccountID(), convertTo<int>(**vnum));
    return 0;
  } else if(**state_form == "saveobj"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Objeditor") << endl;
    cout << head() << body() << endl;
    
    saveObj(cgi, session.getAccountID());
    sendShowObj(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;
  } else if(**state_form == "logout"){
    session.logout();
    cout << HTTPRedirectHeader("objeditor.cgi").setCookie(session.getCookie());
    cout << endl;
    return 0;
  }
  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Objeditor") << endl;
  cout << head() << body() << endl;
  cout << "Fell through state switch.  Bad.<p><hr><p>" << endl;
  cout << **state_form << endl;
  cout << body() << endl;
  cout << html() << endl;
  
  return 0;
}

void delObj(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  db.query("delete from obj where vnum=%s and owner='%s'",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());

  db.query("delete from objextra where vnum=%s and owner='%s'",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());

  db.query("delete from objaffect where vnum=%s and owner='%s'",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());

}

void delExtra(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  db.query("delete from objextra where vnum=%s and owner='%s' and name='%s'",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
           (**(cgi.getElement("name"))).c_str());
}

void makeNewExtra(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }
  
  db.query("insert into objextra (vnum, owner, name, description) values (%s, '%s', '%s', '')",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("name"))).c_str());

}


void makeNewObj(Cgicc cgi, int account_id, bool power_load)
{
  TDatabase db(DB_IMMORTAL);
  TDatabase db_sneezy(DB_SNEEZY);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }
  
  db_sneezy.query("select name, short_desc, long_desc, action_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, volume, material from obj where vnum=%s", (**(cgi.getElement("template"))).c_str());
  db_sneezy.fetchRow();

  db.query("insert into obj (owner, vnum, name, short_desc, long_desc, action_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, volume, material) values ('%s', %s, '%s', '%s', '%s', '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("vnum"))).c_str(),
	   db_sneezy["name"].c_str(), 
	   db_sneezy["short_desc"].c_str(), 
	   db_sneezy["long_desc"].c_str(), 
	   db_sneezy["action_desc"].c_str(), 
	   db_sneezy["type"].c_str(), 
	   db_sneezy["action_flag"].c_str(), 
	   db_sneezy["wear_flag"].c_str(), 
	   db_sneezy["val0"].c_str(), 
	   db_sneezy["val1"].c_str(), 
	   db_sneezy["val2"].c_str(), 
	   db_sneezy["val3"].c_str(), 
	   db_sneezy["weight"].c_str(), 
	   db_sneezy["price"].c_str(), 
	   db_sneezy["can_be_seen"].c_str(), 
	   db_sneezy["spec_proc"].c_str(), 
	   db_sneezy["max_exist"].c_str(), 
	   db_sneezy["max_struct"].c_str(), 
	   db_sneezy["cur_struct"].c_str(), 
	   db_sneezy["decay"].c_str(), 
	   db_sneezy["volume"].c_str(), 
	   db_sneezy["material"].c_str());


  db_sneezy.query("select vnum, name, description from objextra where vnum=%s", (**(cgi.getElement("template"))).c_str());

  while(db_sneezy.fetchRow()){
    db.query("insert into objextra (vnum, owner, name, description) values (%s, '%s', '%s', '%s')", 
	     (**(cgi.getElement("vnum"))).c_str(),
	     (**(cgi.getElement("owner"))).c_str(),
	     db_sneezy["name"].c_str(),
	     db_sneezy["description"].c_str());
  }


  db_sneezy.query("select vnum, type, mod1, mod2 from objaffect where vnum=%s", (**(cgi.getElement("template"))).c_str());

  while(db_sneezy.fetchRow()){
    db.query("insert into objaffect (vnum, owner, type, mod1, mod2) values (%s, '%s', %s, %s, %s)", 
	     (**(cgi.getElement("vnum"))).c_str(),
	     (**(cgi.getElement("owner"))).c_str(),
	     db_sneezy["type"].c_str(),
	     db_sneezy["mod1"].c_str(),
	     db_sneezy["mod2"].c_str());
  }


}

void saveExtra(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }
  
  db.query("delete from objextra where owner='%s' and vnum=%s and name='%s'",
  	   (**(cgi.getElement("owner"))).c_str(), 
  	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("name"))).c_str());

  db.query("insert into objextra (vnum, owner, name, description) values (%s, '%s', '%s', '%s')",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("name"))).c_str(),
	   (**(cgi.getElement("description"))).c_str());
  
  cout << "Saved for keyword " << (**(cgi.getElement("name"))) << ".<br>";
}

void saveObj(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  // calculate action_flag value
  int action_flag=0;
  for(int i=0;i<MAX_OBJ_STAT;++i){
    if(cgi.getElement(extra_bits[i]) != cgi.getElements().end()){
      action_flag|=(1<<i);
    }
  }

  // calculate wear_flag value
  int wear_flag=0;
  for(unsigned int i=0;i<MAX_ITEM_WEARS;++i){
    if(cgi.getElement(wear_bits[i]) != cgi.getElements().end()){
      wear_flag|=(1<<i);
    }
  }



  db.query("delete from obj where owner='%s' and vnum=%s",
  	   (**(cgi.getElement("owner"))).c_str(), 
  	   (**(cgi.getElement("vnum"))).c_str());
  
  db.query("insert into obj (owner, vnum, name, short_desc, long_desc, action_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, volume, material) values ('%s', %s, '%s', '%s', '%s', '%s', %s, %i, %i, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("name"))).c_str(),
	   (**(cgi.getElement("short_desc"))).c_str(),
	   (**(cgi.getElement("long_desc"))).c_str(),
	   (**(cgi.getElement("action_desc"))).c_str(),
	   (**(cgi.getElement("type"))).c_str(),
	   action_flag,
	   wear_flag,
	   (**(cgi.getElement("val0"))).c_str(),
	   (**(cgi.getElement("val1"))).c_str(),
	   (**(cgi.getElement("val2"))).c_str(),
	   (**(cgi.getElement("val3"))).c_str(),
	   (**(cgi.getElement("weight"))).c_str(),
	   (**(cgi.getElement("price"))).c_str(),
	   (**(cgi.getElement("can_be_seen"))).c_str(),
	   (**(cgi.getElement("spec_proc"))).c_str(),
	   (**(cgi.getElement("max_exist"))).c_str(),
	   (**(cgi.getElement("max_struct"))).c_str(),
	   (**(cgi.getElement("cur_struct"))).c_str(),
	   (**(cgi.getElement("decay"))).c_str(),
	   (**(cgi.getElement("volume"))).c_str(),
	   (**(cgi.getElement("material"))).c_str());

  cout << "Saved.<br>";
}

void sendShowExtra(int account_id, int vnum)
{
  TDatabase db(DB_IMMORTAL);

  cout << "<form method=post action=objeditor.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<button name=state value=main type=submit>go back</button>";
  cout << "<p></form>" << endl;

  db.query("select owner from obj where lower(owner) in (%r) and vnum=%i group by owner",
	   getPlayerNames(account_id).c_str(), vnum);
  db.fetchRow();

  cout << "<form method=post action=objeditor.cgi>" << endl;
  cout << "<button name=state value=newextra type=submit>new extra</button>";
  cout << "<input type=text name=name>";
  cout << "<input type=hidden name=vnum value=" << vnum << ">";
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  cout << "</form>";
  cout << endl;
  

  db.query("select owner, vnum, name, description from objextra where vnum=%i and owner in (%r) order by name", vnum, getPlayerNames(account_id).c_str());

  while(db.fetchRow()){
    cout << "<form action=\"objeditor.cgi\" method=post name=saveextra>" << endl;
    cout << "<input type=hidden name=state value=saveextra>" << endl;

    cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
    cout << "<table border=1>";

    cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "vnum" % "vnum" % db["vnum"];
    cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "name" % "name" % db["name"];
    cout << fmt("<tr><td>%s</td><td><textarea name=description cols=90 rows=5>%s</textarea></td></tr>\n") % "description" % db["description"];

    cout << fmt("<tr><td></td><td bgcolor=black>%s</td></tr>\n") %
      mudColorToHTML(db["description"]);
    
    cout << "</table>";    
    cout << "<table width=100%><tr><td align left>";
    cout << "<input type=submit value='save changes'>";
    cout << "</form></td><td width=100% align=right></td><td>";

    cout << "<form method=post action=objeditor.cgi>";
    cout << "<button name=state value=delextra type=submit>delete</button>";
    cout << "<input type=hidden name=name value='" << db["name"] << "'>";
    cout << "<input type=hidden name=vnum value=" << vnum << ">";
    cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
    cout << "</form></td></tr></table>";
    

    cout << "<hr>";
  }


  cout << body() << endl;
  cout << html() << endl;

}

sstring getMaterialForm(int selected)
{
  // all this stl stuff is so I can alphabetical sort material names
  multimap <sstring, int, std::less<sstring> > m;
  multimap <sstring, int, std::less<sstring> >::iterator it;

  // stuff all the materials into the multimap
  for(int i=0;i<200;++i){
    if(material_nums[i].mat_name[0]){
      m.insert(pair<sstring,int>(material_nums[i].mat_name, i));
    }
  }

  sstring buf="<tr><td>material</td><td><select name=material>\n";
  for(it=m.begin();it!=m.end();++it){
    buf+=fmt("<option value=%i %s>%s</option>\n") %
      (*it).second % (((*it).second==selected)?"selected":"") % 
      (*it).first;
  }
  buf+="</select>\n";

  return buf;
}



sstring getTypesForm(int selected)
{
  multimap <sstring, int, std::less<sstring> > m;
  multimap <sstring, int, std::less<sstring> >::iterator it;

  for(int i=0;i<MAX_OBJ_TYPES;++i){
    m.insert(pair<sstring,int>(ItemInfo[i]->name, i));
  }

  sstring buf="<tr><td>type</td><td><select name=type>\n";
  for(it=m.begin();it!=m.end();++it){
    buf+=fmt("<option value=%i %s>%s</option>\n") %
      (*it).second % (((*it).second==selected)?"selected":"") % 
      (*it).first;
  }
  buf+="</select>\n";

  return buf;
}


sstring getProcForm(int selected, bool wizard)
{
  multimap <sstring, int, std::less<sstring> > m;
  multimap <sstring, int, std::less<sstring> >::iterator it;

  m.insert(pair<sstring,int>("-- none", 0));

  for(int i=1;i<NUM_OBJ_SPECIALS-1;++i){
    if(objSpecials[i].name!="BOGUS")
      m.insert(pair<sstring,int>(objSpecials[i].name, i));
  }

  sstring buf="<tr><td>spec_proc</td><td><select name=spec_proc>\n";
  for(it=m.begin();it!=m.end();++it){
    if(objSpecials[(*it).second].assignable || 
       (*it).second==selected ||
       wizard){
      buf+=fmt("<option value=%i %s>%s</option>\n") %
	(*it).second % (((*it).second==selected)?"selected":"") % 
	(*it).first;
    }
  }
  buf+="</select>\n";

  return buf;
}



void sendShowObj(int account_id, int vnum, bool wizard)
{
  TDatabase db(DB_IMMORTAL);

  assign_item_info();

  db.query("select owner, vnum, name, short_desc, long_desc, action_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, volume, material from obj where vnum=%i and owner in (%r)", vnum, getPlayerNames(account_id).c_str());
  db.fetchRow();


  cout << "<form method=post action=objeditor.cgi>" << endl;
  cout << "<table width=100%><tr>";
  cout << "<td align=left><button name=state value=logout type=submit>logout</button></td>";
  cout << "<td align=left><button name=state value=main type=submit>go back</button></td>";
  cout << "<td width=100% align=right><button name=state value=delobj type=submit>delete</button></td>";
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  cout << "<input type=hidden name=vnum value='" << vnum << "'>";
  cout << "<p></form>" << endl;

  cout << "<form action=\"objeditor.cgi\" method=post name=saveobj>" << endl;
  cout << "<input type=hidden name=state value=saveobj>" << endl;

  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";


  cout << "<table border=1>";


  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "vnum" % "vnum" % db["vnum"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "name" % "name" % db["name"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "short_desc" % "short_desc" % db["short_desc"];

  cout << fmt("<tr><td></td><td bgcolor=black>%s</td></tr>\n") % 
    mudColorToHTML(db["short_desc"]);

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "long_desc" % "long_desc" % db["long_desc"];

  cout << fmt("<tr><td></td><td bgcolor=black>%s</td></tr>\n") % 
    mudColorToHTML(db["long_desc"]);

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "action_desc" % "action_desc" % db["action_desc"];

  cout << getTypesForm(convertTo<int>(db["type"]));

  // action flag
  cout << "<tr><td>action_flag</td><td><table><tr>" << endl;
  int action_flag=convertTo<int>(db["action_flag"]);
  for(int i=0;i<MAX_OBJ_STAT;++i){
    cout << fmt("<td><input type=checkbox %s name='%s'> %s") %
      ((action_flag & (1<<i))?"checked":"") % extra_bits[i] % extra_bits[i];

    cout << "</td>";

    if(!((i+1) % 6))
      cout << "</tr><tr>";
  }
  cout <<"</tr></table></td></tr>";
  //


  // wear flag
  cout << "<tr><td>wear_flag</td><td><table><tr>" << endl;
  int wear_flag=convertTo<int>(db["wear_flag"]);
  for(unsigned int i=0;i<MAX_ITEM_WEARS;++i){
    cout << fmt("<td><input type=checkbox %s name='%s'> %s") %
      ((wear_flag & (1<<i))?"checked":"") % wear_bits[i] % wear_bits[i];

    cout << "</td>";

    if(!((i+1) % 8))
      cout << "</tr><tr>";
  }
  cout <<"</tr></table></td></tr>";
  //

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % ItemInfo[convertTo<int>(db["type"])]->val0_info % "val0" % db["val0"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % ItemInfo[convertTo<int>(db["type"])]->val1_info % "val1" % db["val1"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % ItemInfo[convertTo<int>(db["type"])]->val2_info % "val2" % db["val2"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % ItemInfo[convertTo<int>(db["type"])]->val3_info % "val3" % db["val3"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "weight" % "weight" % db["weight"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "price" % "price" % db["price"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "can_be_seen" % "can_be_seen" % db["can_be_seen"];

  cout << getProcForm(convertTo<int>(db["spec_proc"]), wizard);

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "max_exist" % "max_exist" % db["max_exist"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "max_struct" % "max_struct" % db["max_struct"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "cur_struct" % "cur_struct" % db["cur_struct"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "decay" % "decay" % db["decay"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "volume" % "volume" % db["volume"];


  cout << getMaterialForm(convertTo<int>(db["material"]));
  

  cout << "</table>";

  cout << "<input type=submit value='save changes'>";
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
  
  
}

void sendObjlist(int account_id){
  TDatabase db(DB_IMMORTAL);
  TDatabase db_affects(DB_IMMORTAL);
  TDatabase db_extras(DB_IMMORTAL);

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Objeditor") << endl;
  sendJavaScript();
  cout << head() << body() << endl;

  cout << "<form method=post action=objeditor.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<p></form>";
 
  db.query("select owner, max(vnum)+1 as nvnum from obj where lower(owner) in (%r) group by owner",
	   getPlayerNames(account_id).c_str());
  db.fetchRow();

  cout << "<form method=post action=objeditor.cgi>" << endl;
  cout << "<button name=state value=newobj type=submit>new obj</button>";
  cout << "vnum <input type=text name=vnum value=" << db["nvnum"] << ">";
  cout << "template <input type=text name=template value=1>";
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  cout << "</form>";
  cout << endl;

  cout << "<form action=\"objeditor.cgi\" method=post name=pickobj>" << endl;
  cout << "<input type=hidden name=vnum>" << endl;
  cout << "<input type=hidden name=state>" << endl;

  cout << "<table border=1>";
  cout << "<tr><td>vnum</td><td>name</td><td>extras</td><td>affects</td></tr>";

  db.query("select vnum, name from obj o where lower(owner) in (%r) order by vnum asc", getPlayerNames(account_id).c_str());
  
  db_affects.query("select vnum, count(*) as count from objaffect where lower(owner) in (%r) group by vnum order by vnum asc", getPlayerNames(account_id).c_str());
  db_affects.fetchRow();

  db_extras.query("select vnum, count(*) as count from objextra where lower(owner) in (%r) group by vnum order by vnum asc", getPlayerNames(account_id).c_str());
  db_extras.fetchRow();

  int affcount=0, extracount=0;
  while(db.fetchRow()){
    affcount=extracount=0;
    while(convertTo<int>(db_affects["vnum"]) < 
	  convertTo<int>(db["vnum"]))
      if(!db_affects.fetchRow())
	break;
    while(convertTo<int>(db_extras["vnum"]) < 
	  convertTo<int>(db["vnum"]))
      if(!db_extras.fetchRow())
	break;
    
    if(db_affects["vnum"]==db["vnum"]){
      affcount=convertTo<int>(db_affects["count"]);
      db_affects.fetchRow();
    }
    if(db_extras["vnum"]==db["vnum"]){
      extracount=convertTo<int>(db_extras["count"]);
      db_extras.fetchRow();
    }

    cout << "<tr><td>" << "<a href=javascript:pickobj('" << db["vnum"];
    cout << "','showobj')>" << db["vnum"] << "</a>" << endl;
    cout << "</td><td>" << db["name"] << "</td>"<< endl;
    cout << "<td><a href=javascript:pickobj('" << db["vnum"];
    cout << "','showextra')>" << extracount << "</a></td>" << endl;
    cout << "<td><a href=javascript:pickobj('" << db["vnum"];
    cout << "','showaffect')>" << affcount << "</a></td>" << endl;
    cout << "</tr>" << endl;

  }

  cout << "</table></form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}


void sendJavaScript()
{
  cout << "<script language=\"JavaScript\" type=\"text/javascript\">" << endl;
  cout << "<!--" << endl;

  cout << "function pickobj(vnum, state)" << endl;
  cout << "{" << endl;
  cout << "document.pickobj.state.value = state;" << endl;
  cout << "document.pickobj.vnum.value = vnum;" << endl;
  cout << "document.pickobj.submit();" << endl;
  cout << "}" << endl;

  cout << "-->" << endl;
  cout << "</script>" << endl;


}

// candidate for inclusion in sstring
void replaceString(sstring &str, sstring find, sstring replace)
{
  while(str.find(find)!=sstring::npos){
    str.replace(str.find(find), find.size(), replace);
  }
}

// candidate for some sort of global cgi tools library
sstring mudColorToHTML(sstring str)
{

  replaceString(str, "\n", "<br>");

  replaceString(str, "<f>", "");
  replaceString(str, " ", "&nbsp;");
  replaceString(str, "<r>", "</span><span style=\"color:red\">");
  replaceString(str, "<R>", "</span><span style=\"color:red;font-weight:bold\">");

  replaceString(str, "<b>", "</span><span style=\"color:blue\">");
  replaceString(str, "<B>", "</span><span style=\"color:blue;font-weight:bold\">");
  replaceString(str, "<g>", "</span><span style=\"color:green\">");
  replaceString(str, "<G>", "</span><span style=\"color:green;font-weight:bold\">");
  replaceString(str, "<c>", "</span><span style=\"color:cyan\">");
  replaceString(str, "<C>", "</span><span style=\"color:cyan;font-weight:bold\">");
  replaceString(str, "<p>", "</span><span style=\"color:purple\">");
  replaceString(str, "<P>", "</span><span style=\"color:purple;font-weight:bold\">");
  replaceString(str, "<o>", "</span><span style=\"color:orange\">");
  replaceString(str, "<O>", "</span><span style=\"color:orange;font-weight:bold\">");
  replaceString(str, "<y>", "</span><span style=\"color:yellow\">");
  replaceString(str, "<Y>", "</span><span style=\"color:yellow;font-weight:bold\">");
  replaceString(str, "<k>", "</span><span style=\"color:gray\">");
  replaceString(str, "<K>", "</span><span style=\"color:gray;font-weight:bold\">");
  replaceString(str, "<w>", "</span><span style=\"color:white\">");
  replaceString(str, "<W>", "</span><span style=\"color:white;font-weight:bold\">");
  replaceString(str, "<Z>", "</span><span style=\"color:white\">");
  replaceString(str, "<z>", "</span><span style=\"color:white\">");
  replaceString(str, "<1>", "</span><span style=\"color:white\">");

  // to help builders line up text
  sstring spacing_strip="01234567890123456789012345678901234567890123456789012345678901234567890123456789";

  return fmt("<span style=\"color:white\"><font face=\"courier\">%s<br>%s</font></span>") % spacing_strip % str;
}

