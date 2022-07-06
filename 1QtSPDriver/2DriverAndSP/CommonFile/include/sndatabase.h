#ifndef SNDATABASE_H
#define SNDATABASE_H

#include <string>
#include <vector>
#include "SNDatabase_global.h"

typedef struct _sndb_note_info {
    std::string serial_number;
    std::string currency_id;
    int note_id;
    int value;
    std::string note_ver;
    std::string origin;
    std::string destination;
    std::string category;
    std::string reject_cause;
    std::string sn_image_path;
    std::string full_image_path1;
    std::string full_image_path2;
} SNDBNoteInfo, *LPSNDBNoteInfo;

typedef struct _sndb_time_info {
    unsigned short year;
    unsigned short month;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
    unsigned short milli_second;
} SNDBTimeInfo, *LPSNDBTimeInfo;

typedef struct _sndb_info {
    std::string act_name;
    SNDBTimeInfo act_time;
    int num_of_notes;

    std::vector<SNDBNoteInfo> notes_info;
} SNDBInfo, *LPSNDBInfo;

struct sqlite3;
class SNDATABASE_EXPORT SNDatabase
{
public:
    SNDatabase();
    SNDatabase(std::string db_path);
    virtual ~SNDatabase();

public:
    void close_db();
    bool init_db(std::string db_path, bool recreate = false);
    bool save_sndb_info(SNDBInfo& sndb_info);
    bool delete_sndb_info(SNDBTimeInfo& deadline_time);

protected:
    static int exed_callback(void* param, int count, char** values, char** keys);
    bool insert_into_ActInfo_Table(
            std::string act_id, std::string act_name, std::string act_time, int num_of_notes);
    bool insert_into_NoteInfo_Table(
            std::string act_id, std::string serial_number, std::string currency_id, int note_id, int value, std::string note_ver,
            std::string origin, std::string destination, std::string category, std::string reject_cause,
            std::string sn_image_path, std::string full_image_path1 = "", std::string full_image_path2 = "");


protected:
    sqlite3* m_db;
};

#endif // SNDATABASE_H
