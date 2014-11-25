#include "smCore/smConfig.h"
#include <QString>

smString &smString::operator=(smChar *p_param){
	QString::operator=(p_param);
	return *this;
}

ostream &operator<<(ostream &p_os, smString &p_param){
	p_os<<p_param.toAscii().data();
	return p_os;
}
