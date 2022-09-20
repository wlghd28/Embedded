#include "SearchMolrarity.h"

double SearchMolrarity
(
    DATABASE* Database,
    int iLength_Database,
    double dbTolerance_Temp,
    double dbTolerance_Density,
    double dbTemp,
    double dbDensity
)
{
    if(dbTolerance_Temp < 0 || dbTolerance_Density < 0) { return -1; }

    double dbOffset = 0;
    double dbOffset_Temp = 99;
    double dbOffset_Density = 99;

    // Offset 값 (입력 받은 값 - DB 값) 이 최소가 될 때마다 값을 갱신하고 그 인덱스의 온도, 농도 값을 저장해놓는다.
    double dbMinOffset_Temp = 0;
    double dbMinOffset_Molrarity = 0;

    // 최소오차 온도 값 찾기
    for (int i = 0; i < iLength_Database; i++)
    {
        dbOffset = dbTemp - Database[i].m_dbTemp;
        //printf("%lf %lf\n", dbTemp, Database[i].m_dbTemp);

        if (dbOffset < 0) dbOffset *= -1;

        //printf("%lf\n", dbOffset);
        if (dbOffset < dbOffset_Temp)
        {
            dbOffset_Temp = dbOffset;
            dbMinOffset_Temp = Database[i].m_dbTemp;
        }
    }

    if (dbOffset_Temp > dbTolerance_Temp) { return -1; }

    // 찾은 최소오차 온도에 해당하는 밀도 값중 최소오차 밀도 값 찾기
    dbOffset = 0;
    for (int i = 0; i < iLength_Database; i++)
    {
        if (Database[i].m_dbTemp == dbMinOffset_Temp)
        {
            dbOffset = dbDensity - Database[i].m_dbDensity;
            if (dbOffset < 0) dbOffset *= -1;

            if (dbOffset < dbOffset_Density)
            {
                dbOffset_Density = dbOffset;
                dbMinOffset_Molrarity = Database[i].m_dbMolrarity;
            }
        }
    }

    if (dbOffset_Density > dbTolerance_Density) { return -1; }

    return dbMinOffset_Molrarity;
}
