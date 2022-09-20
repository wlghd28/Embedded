// 몰농도 서칭
#ifndef _SEARCHMOLRARITY_H_
#define _SEARCHMOLRARITY_H_

#include "Database.h"

double SearchMolrarity
(
    DATABASE* Database,
    int iLength_Database,
    double dbTolerance_Temp,
    double dbTolerance_Density,
    double dbTemp,
    double dbDensity
);

#endif
